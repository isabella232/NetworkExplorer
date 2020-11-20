#include "pch.h"
#include "ActiveConnectionTracker.h"

#pragma comment(lib, "Iphlpapi")

int ActiveConnectionTracker::EnumConnections() {
	DWORD size = 1 << 16;
	auto orgSize = size;
	auto buffer = ::VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (!buffer)
		return -1;

	bool first = _connections.empty();
	_newConnections.clear();
	_closedConnections.clear();
	if (first) {
		_connectionMap.reserve(512);
		_newConnections.reserve(16);
		_closedConnections.reserve(16);
	}

	auto map = _connectionMap;
	std::vector<std::shared_ptr<Connection>> local;
	local.reserve(256);
	if ((_trackedConnections & ConnectionType::Tcp) == ConnectionType::Tcp) {
		if(::GetExtendedTcpTable(buffer, &size, FALSE, AF_INET, TCP_TABLE_OWNER_MODULE_ALL, 0) == NOERROR) {
			auto table = (PMIB_TCPTABLE_OWNER_MODULE)buffer;
			AddTcp4Connections(table, map, local, first);
		}
	}
	if ((_trackedConnections & ConnectionType::TcpV6) == ConnectionType::TcpV6) {
		size = orgSize;
		if (::GetExtendedTcpTable(buffer, &size, FALSE, AF_INET6, TCP_TABLE_OWNER_MODULE_ALL, 0) == NOERROR) {
			auto table = (PMIB_TCP6TABLE_OWNER_MODULE)buffer;
			AddTcp6Connections(table, map, local, first);
		}
	}
	if ((_trackedConnections & ConnectionType::Udp) == ConnectionType::Udp) {
		size = orgSize;
		if (::GetExtendedUdpTable(buffer, &size, FALSE, AF_INET, UDP_TABLE_OWNER_MODULE, 0) == NOERROR) {
			auto table = (PMIB_UDPTABLE_OWNER_MODULE)buffer;
			AddUdp4Connections(table, map, local, first);
		}
	}

	if ((_trackedConnections & ConnectionType::UdpV6) == ConnectionType::UdpV6) {
		size = orgSize;
		if (::GetExtendedUdpTable(buffer, &size, FALSE, AF_INET6, UDP_TABLE_OWNER_MODULE, 0) == NOERROR) {
			auto table = (PMIB_UDP6TABLE_OWNER_MODULE)buffer;
			AddUdp6Connections(table, map, local, first);
		}
	}

	for (const auto& [key, conn] : map) {
		_closedConnections.push_back(conn);
		_connectionMap.erase(*conn);
	}
	if (!first) {
		_connections = std::move(local);
	}
	::VirtualFree(buffer, 0, MEM_RELEASE);

	return static_cast<int>(_connections.size());
}

void ActiveConnectionTracker::SetTrackingFlags(ConnectionType type) {
	_trackedConnections = type;
}

ConnectionType ActiveConnectionTracker::GetTrackingFlags() const {
	return _trackedConnections;
}

const std::vector<std::shared_ptr<Connection>>& ActiveConnectionTracker::GetConnections() const {
	return _connections;
}

const std::vector<std::shared_ptr<Connection>>& ActiveConnectionTracker::GetNewConnections() const {
	return _newConnections;
}

const std::vector<std::shared_ptr<Connection>>& ActiveConnectionTracker::GetClosedConnections() const {
	return _closedConnections;
}

void ActiveConnectionTracker::AddTcp4Connections(PMIB_TCPTABLE_OWNER_MODULE table, ConnectionMap& map, ConnectionVec& local, bool first) {
	auto count = table->dwNumEntries;
	const auto& items = table->table;

	for (DWORD i = 0; i < count; i++) {
		const auto& item = items[i];
		std::shared_ptr<Connection> conn;
		if (first) {
			conn = std::make_shared<Connection>();
			conn->Pid = item.dwOwningPid;
			conn->TimeStamp = item.liCreateTimestamp.QuadPart;		
			conn->LocalPort = item.dwLocalPort;
			conn->LocalAddress = item.dwLocalAddr;
			conn->RemoteAddress = item.dwRemoteAddr;
			conn->RemotePort = item.dwRemotePort;
			conn->Type = ConnectionType::Tcp;
			conn->State = (MIB_TCP_STATE)item.dwState;
			_connections.push_back(conn);
			_connectionMap.insert({ *conn, conn });
		}
		else {
			Connection key;
			key.LocalAddress = item.dwLocalAddr;
			key.RemoteAddress = item.dwRemoteAddr;
			key.LocalPort = item.dwLocalPort;
			key.RemotePort = item.dwRemotePort;
			key.Pid = item.dwOwningPid;
			key.Type = ConnectionType::Tcp;

			if (auto it = map.find(key); it != map.end()) {
				conn = it->second;
				map.erase(key);
			}
			else {
				conn = std::make_shared<Connection>();
				*conn = key;
				conn->TimeStamp = item.liCreateTimestamp.QuadPart;
				_connectionMap.insert({ *conn, conn });
				_newConnections.push_back(conn);
			}
			conn->State = (MIB_TCP_STATE)item.dwState;
			local.push_back(conn);
		}
	}
}

void ActiveConnectionTracker::AddTcp6Connections(PMIB_TCP6TABLE_OWNER_MODULE table, ConnectionMap& map, ConnectionVec& local, bool first) {
	auto count = table->dwNumEntries;
	const auto& items = table->table;

	for (DWORD i = 0; i < count; i++) {
		const auto& item = items[i];
		std::shared_ptr<Connection> conn;
		if (first) {
			conn = std::make_shared<Connection>();
			conn->Pid = item.dwOwningPid;
			conn->LocalPort = item.dwLocalPort;
			::memcpy(conn->ucLocalAddrress, item.ucLocalAddr, sizeof(conn->ucLocalAddrress));
			::memcpy(conn->ucRemoteAddrress, item.ucRemoteAddr, sizeof(conn->ucRemoteAddrress));
			conn->RemotePort = item.dwRemotePort;
			conn->Type = ConnectionType::TcpV6;
			conn->State = (MIB_TCP_STATE)item.dwState;
			conn->TimeStamp = item.liCreateTimestamp.QuadPart;
			_connections.push_back(conn);
			_connectionMap.insert({ *conn, conn });
		}
		else {
			Connection key;
			key.LocalPort = item.dwLocalPort;
			::memcpy(key.ucLocalAddrress, item.ucLocalAddr, sizeof(key.ucLocalAddrress));
			::memcpy(key.ucRemoteAddrress, item.ucRemoteAddr, sizeof(key.ucRemoteAddrress));
			key.RemotePort = item.dwRemotePort;
			key.Pid = item.dwOwningPid;
			key.Type = ConnectionType::TcpV6;

			if (auto it = map.find(key); it != map.end()) {
				conn = it->second;
				map.erase(key);
			}
			else {
				conn = std::make_shared<Connection>();
				*conn = key;
				conn->TimeStamp = item.liCreateTimestamp.QuadPart;
				_connectionMap.insert({ *conn, conn });
				_newConnections.push_back(conn);
			}
			conn->State = (MIB_TCP_STATE)item.dwState;
			local.push_back(conn);
		}
	}
}

void ActiveConnectionTracker::AddUdp4Connections(PMIB_UDPTABLE_OWNER_MODULE table, ConnectionMap& map, ConnectionVec& local, bool first) {
	auto count = table->dwNumEntries;
	const auto& items = table->table;

	for (DWORD i = 0; i < count; i++) {
		const auto& item = items[i];
		std::shared_ptr<Connection> conn;
		if (first) {
			conn = std::make_shared<Connection>();
			conn->LocalPort = item.dwLocalPort;
			conn->LocalAddress = item.dwLocalAddr;
			conn->Pid = item.dwOwningPid;
			conn->Type = ConnectionType::Udp;
			conn->State = (MIB_TCP_STATE)0;
			conn->TimeStamp = item.liCreateTimestamp.QuadPart;
			_connections.push_back(conn);
			_connectionMap.insert({ *conn, conn });
		}
		else {
			Connection key;
			key.LocalAddress = item.dwLocalAddr;
			key.LocalPort = item.dwLocalPort;
			key.Pid = item.dwOwningPid;
			key.Type = ConnectionType::Udp;

			if (auto it = map.find(key); it != map.end()) {
				conn = it->second;
				map.erase(key);
			}
			else {
				conn = std::make_shared<Connection>();
				*conn = key;
				conn->TimeStamp = item.liCreateTimestamp.QuadPart;
				_connectionMap.insert({ key, conn });
				_newConnections.push_back(conn);
			}
			local.push_back(conn);
		}
	}
}

void ActiveConnectionTracker::AddUdp6Connections(PMIB_UDP6TABLE_OWNER_MODULE table, ConnectionMap& map, ConnectionVec& local, bool first) {
	auto count = table->dwNumEntries;
	const auto& items = table->table;

	for (DWORD i = 0; i < count; i++) {
		const auto& item = items[i];
		std::shared_ptr<Connection> conn;
		if (first) {
			conn = std::make_shared<Connection>();
			conn->LocalPort = item.dwLocalPort;
			::memcpy(conn->ucLocalAddrress, item.ucLocalAddr, sizeof(item.ucLocalAddr));
			conn->Pid = item.dwOwningPid;
			conn->Type = ConnectionType::UdpV6;
			conn->State = (MIB_TCP_STATE)0;
			conn->TimeStamp = item.liCreateTimestamp.QuadPart;
			_connections.push_back(conn);
			_connectionMap.insert({ *conn, conn });
		}
		else {
			Connection key;
			::memcpy(key.ucLocalAddrress, item.ucLocalAddr, sizeof(item.ucLocalAddr));
			key.LocalPort = item.dwLocalPort;
			key.Pid = item.dwOwningPid;
			key.Type = ConnectionType::UdpV6;

			if (auto it = map.find(key); it != map.end()) {
				conn = it->second;
				map.erase(key);
			}
			else {
				conn = std::make_shared<Connection>();
				*conn = key;
				conn->TimeStamp = item.liCreateTimestamp.QuadPart;
				_connectionMap.insert({ key, conn });
				_newConnections.push_back(conn);
			}
			local.push_back(conn);
		}
	}
}

bool Connection::operator==(const Connection& other) const {
	return
		Type == other.Type &&
		LocalAddress == other.LocalAddress &&
		LocalPort == other.LocalPort &&
		Pid == other.Pid;
}
