#pragma once

#include <iphlpapi.h>
#include <functional>

enum class ConnectionType : DWORD {
	Invalid = 0,
	Tcp = 1, TcpV6 = 2, Udp = 4, UdpV6 = 8,
	All = 15
};
DEFINE_ENUM_FLAG_OPERATORS(ConnectionType);

struct Connection {
	MIB_TCP_STATE State{};
	DWORD Pid;
	ConnectionType Type;
	union {
		DWORD LocalAddress;
		IN6_ADDR LocalAddressV6;
		UCHAR ucLocalAddrress[16]{ 0 };
	};
	union {
		DWORD RemoteAddress;
		IN6_ADDR RemoteAddressV6;
		UCHAR ucRemoteAddrress[16]{ 0 };
	};
	DWORD LocalPort;
	DWORD RemotePort{ 0 };
	LONGLONG TimeStamp{ 0 };

	bool operator==(const Connection& other) const;
};

template<>
struct std::hash<Connection> {
	const size_t operator()(const Connection& conn) const {
		return conn.Pid ^ conn.LocalAddress ^ (conn.LocalPort << 6) ^ ((int)conn.Type << 20) ^ conn.RemoteAddress;
	}
};

class ActiveConnectionTracker {
public:
	int EnumConnections();
	void Reset();

	void SetTrackingFlags(ConnectionType type);
	ConnectionType GetTrackingFlags() const;

	using ConnectionMap = std::unordered_map<Connection, std::shared_ptr<Connection>>;
	using ConnectionVec = std::vector<std::shared_ptr<Connection>>;

	const ConnectionVec& GetConnections() const;
	const ConnectionVec& GetNewConnections() const;
	const ConnectionVec& GetClosedConnections() const;

private:
	void AddTcp4Connections(PMIB_TCPTABLE_OWNER_MODULE table, ConnectionMap& map, ConnectionVec& local, bool first);
	void AddTcp6Connections(PMIB_TCP6TABLE_OWNER_MODULE table, ConnectionMap& map, ConnectionVec& local, bool first);
	void AddUdp4Connections(PMIB_UDPTABLE_OWNER_MODULE table, ConnectionMap& map, ConnectionVec& local, bool first);
	void AddUdp6Connections(PMIB_UDP6TABLE_OWNER_MODULE table, ConnectionMap& map, ConnectionVec& local, bool first);

	ConnectionType _trackedConnections{ ConnectionType::All };
	ConnectionVec _connections;
	ConnectionVec _newConnections;
	ConnectionVec _closedConnections;
	ConnectionMap _connectionMap;
};

