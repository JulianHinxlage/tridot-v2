//
// Copyright (c) 2022 Julian Hinxlage. All rights reserved.
//

#pragma once

#include "pch.h"
#include "TcpSocket.h"
#include "Packet.h"

namespace tri {

	class Connection {
	public:
		std::vector<uint8_t> buffer;
		Ref<TcpSocket> socket;
		int threadId;
		std::mutex reconnectMutex;
		std::condition_variable reconnect;
		bool running;
		std::function<void(Connection* conn)> onDisconnect;
		std::function<void(Connection* conn)> onConnect;
		std::function<void(Connection* conn)> onFail;

		Connection();
		~Connection();
		void run(const std::function<void(Connection* conn, void* data, int bytes)>& callback);
		void runConnect(const std::string &address, uint16_t port, const std::function<void(Connection* conn, void* data, int bytes)>& callback);
		void runListen(uint16_t port, const std::function<void(Ref<Connection> conn)>& callback);
		void stop();

		bool write(const void* data, int bytes);
		bool write(Packet &packet);
	private:
		void runImpl(const std::function<void(Connection* conn, void* data, int bytes)>& callback);
	};

}
