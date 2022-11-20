#include "NetworkManager.h"
#include "NetworkManager.h"
//
// Copyright (c) 2022 Julian Hinxlage. All rights reserved.
//

#include "NetworkManager.h"
#include "core/core.h"
#include "engine/RuntimeMode.h"
#include "Packet.h"
#include "NetworkReplication.h"
#include "engine/EntityUtil.h"
#include "NetworkComponent.h"

#if TRI_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netdb.h>
#include<arpa/inet.h>
#endif

namespace tri {
	
	TRI_SYSTEM_INSTANCE(NetworkManager, env->networkManager);

	TRI_CLASS(NetOpcode);
	TRI_ENUM8(NetOpcode, NOOP, MAP_REQUEST, MAP_RESPONSE, MAP_LOADED, ENTITY_ADD, ENTITY_UPDATE, ENTITY_REMOVE, ENTITY_OWNING);

	void NetworkManager::init() {
		env->runtimeMode->setActiveSystem<NetworkManager>({ RuntimeMode::LOADING, RuntimeMode::EDIT, RuntimeMode::PAUSED }, true);
		env->jobManager->addJob("Network")->addSystem<NetworkManager>();

		mode = STANDALONE;
		strMode = "";
		serverAddress = "localhost";
		serverPort = 24052;

		env->console->addCVar("serverAdress", &serverAddress);
		env->console->addCVar("serverPort", &serverPort);
		env->console->addCVar<std::string>("networkMode", "standalone");
	}

	void NetworkManager::startup() {
#if TRI_WINDOWS
		WSADATA wsaData;
		int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (result != NO_ERROR) {
			env->console->error("WSA startup failed with code %i", result);
			return;
		}
#endif
	}

	void NetworkManager::tick() {
		std::string strMode = env->console->getCVarValue<std::string>("networkMode", "standalone");
		strMode = StrUtil::toLower(strMode);
		if (strMode != this->strMode) {
			if (strMode == "standalone") {
				this->strMode = strMode;
				setMode(STANDALONE);
			}
			else if (strMode == "client") {
				this->strMode = strMode;
				setMode(CLIENT);
			}
			else if (strMode == "server") {
				this->strMode = strMode;
				setMode(SERVER);
			}
			else if (strMode == "host") {
				this->strMode = strMode;
				setMode(HOST);
			}
		}

		if (mode == CLIENT) {
			if (tryReconnect) {
				setMode(CLIENT);
				tryReconnect = false;
			}
		}

		disconnectedConnections.clear();
	}

	void NetworkManager::shutdown() {
		if (connection) {
			connection->stop();
		}
		for (auto& conn : connections) {
			conn->stop();
		}
		connections.clear();
		disconnectedConnections.clear();

#if TRI_WINDOWS
		WSACleanup();
#endif
	}

	void NetworkManager::setMode(NetMode mode) {
		this->mode = mode;

		if (mode == STANDALONE) {
			if (connection) {
				connection->stop();
				connection = nullptr;
				connections.clear();
			}
		}
		else if (mode == CLIENT) {
			if (!connection) {
				connection = Ref<Connection>::make();
			}
			else {
				if (!tryReconnect) {
					connection->stop();
				}
			}

			connection->onConnect = [&](Connection* conn) {
				env->console->info("connected to %s %i", conn->socket->getEndpoint().getAddress().c_str(), conn->socket->getEndpoint().getPort());
				onConnect.invoke(conn);
			};
			connection->onDisconnect = [&](Connection* conn) {
				env->console->info("disconnected from %s %i", conn->socket->getEndpoint().getAddress().c_str(), conn->socket->getEndpoint().getPort());
				onDisconnect.invoke(conn);
				tryReconnect = true;
			};
			connection->onFail = [&](Connection* conn) {
				env->console->error("failed to connect to %s %i", serverAddress.c_str(), serverPort);
				tryReconnect = true;
			};
			if (tryReconnect) {
				connection->reconnect.notify_one();
			}
			else {
				connection->runConnect(serverAddress, serverPort, [&](Connection* conn, void* data, int bytes) {
					onRead(conn, data, bytes);
				});
			}
		}
		else if (mode == SERVER || mode == HOST) {
			if (!connection) {
				connection = Ref<Connection>::make();
			}
			else {
				connection->stop();
			}

			connection->onFail = [&](Connection* conn) {
				env->console->error("failed to listen on port %i", serverPort);
			};
			connection->onConnect = [&](Connection* conn) {
				env->console->info("listen on port %i", serverPort);
			};
			connection->runListen(serverPort, [&](Ref<Connection> conn) {
				connections.push_back(conn);
				conn->onDisconnect = [&](Connection* conn) {
					env->console->info("disconnect from %s %i", conn->socket->getEndpoint().getAddress().c_str(), conn->socket->getEndpoint().getPort());
					onDisconnect.invoke(conn);
					for (int i = 0; i < connections.size(); i++) {
						if (connections[i].get() == conn) {
							disconnectedConnections.push_back(connections[i]);
							connections.erase(connections.begin() + i);
							break;
						}
					}
				};
				env->console->info("connection from %s %i", conn->socket->getEndpoint().getAddress().c_str(), conn->socket->getEndpoint().getPort());
				onConnect.invoke(conn.get());
				conn->run([&](Connection* conn, void* data, int bytes) {
					onRead(conn, data, bytes);
				});
			});
		}
	}

	bool NetworkManager::isConnected() {
		if (mode == CLIENT) {
			return connection->socket->isConnected();
		}
		else if (mode == SERVER || mode == HOST) {
			return connections.size() > 0;
		}
		return false;
	}

	bool NetworkManager::hasAuthority() {
		return mode != CLIENT;
	}

	void NetworkManager::sendToAll(const void* data, int bytes) {
		if (mode == CLIENT) {
			connection->socket->write(data, bytes);
		}
		else if (mode == SERVER || mode == HOST) {
			for (auto& conn : connections) {
				conn->socket->write(data, bytes);
			}
		}
	}

	void NetworkManager::sendToAll(Packet& packet) {
		sendToAll(packet.data(), packet.size());
	}

	void NetworkManager::onRead(Connection* conn, void* data, int bytes) {
		Packet packet;
		packet.add(data, bytes);
		NetOpcode opcode = packet.get<NetOpcode>();

		env->console->trace("packet with opcode %s %s", EntityUtil::enumString(opcode), packet.get<Guid>().toString().c_str());
		packet.unskip(sizeof(Guid));

		auto entry = packetCallbacks.find(opcode);
		if (entry != packetCallbacks.end()) {
			entry->second(conn, opcode, packet);
		}
		else {
			if (opcode != NOOP) {
				env->console->warning("invalid opcode %i", opcode);
			}
		}
	}

}
