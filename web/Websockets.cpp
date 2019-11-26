#include "stdafx.h"
#include "Websockets.hpp"
#include "../json/json.h"
#include "../main/Logger.h"
#include "../main/mainworker.h"
#include "../main/SQLHelper.h"

#define FIN_MASK 0x80
#define RSVI1_MASK 0x40
#define RSVI2_MASK 0x20
#define RSVI3_MASK 0x10
#define OPCODE_MASK 0x0f
#define MASKING_MASK 0x80
#define PAYLOADLEN_MASK 0x7f

namespace http {
	namespace server {

		CWebsocketFrame::CWebsocketFrame() {
			fin = false;
			rsvi1 = rsvi2 = rsvi3 = false;
			opcode = opcode_continuation;
			masking = false;
			payloadlen = 0;
			bytes_consumed = 0;
		};
		CWebsocketFrame::~CWebsocketFrame() {};

		std::string CWebsocketFrame::unmask(const uint8_t *mask, const uint8_t *bytes, size_t payloadlen) {
			std::string result;
			result.resize(payloadlen);
			for (size_t i = 0; i < payloadlen; i++) {
				result[i] = (uint8_t)(bytes[i] ^ mask[i % 4]);
			}
			return result;
		}

		std::string CWebsocketFrame::Create(opcodes opcode, const std::string &payload, bool domasking)
		{
			size_t_t payloadlen = payload.length();
			std::string res;
			// byte 0
			res += ((uint8_t)opcode | FIN_MASK);
			if (payloadlen < 126) {
				res += (uint8_t)payloadlen | (domasking ? MASKING_MASK : 0);
			}
			else {
				if (payloadlen <= 0xffff) {
					res += (uint8_t)126 | (domasking ? MASKING_MASK : 0);
					int bits = 16;
					while (bits) {
						bits -= 8;
						res += (uint8_t)((payloadlen >> bits) & 0xff);
					}
				}
				else {
					res += (uint8_t)127 | (domasking ? MASKING_MASK : 0);
					int bits = 64;
					while (bits) {
						bits -= 8;
						uint8_t ch = (uint8_t)((size_t)(payloadlen >> bits) & 0xff);
						res += ch;
					}
				}
			}
			if (domasking) {
				// masking key
				uint8_t masking_key[4];
				for (uint8_t i = 0; i < 4; i++) {
					masking_key[i] = rand();
					res += masking_key[i];
				}
				res += unmask(masking_key, (const uint8_t *)payload.c_str(), (size_t)payloadlen);
			}
			else {
				res += payload;
			}
			return res;
		}

		bool CWebsocketFrame::Parse(const uint8_t *bytes, size_t size) {
			uint8_t masking_key[4];
			size_t remaining = size;
			bytes_consumed = 0;
			if (remaining < 2) {
				return false;
			}
			fin = (bytes[0] & FIN_MASK) > 0;
			rsvi1 = (bytes[0] & RSVI1_MASK) > 0;
			rsvi2 = (bytes[0] & RSVI2_MASK) > 0;
			rsvi3 = (bytes[0] & RSVI3_MASK) > 0;
			opcode = (opcodes)(bytes[0] & OPCODE_MASK);
			masking = (bytes[1] & MASKING_MASK) > 0;
			payloadlen = (bytes[1] & PAYLOADLEN_MASK);
			remaining -= 2;
			size_t ptr = 2;
			if (payloadlen == 126) {
				if (remaining < 2) {
					return false;
				}
				payloadlen = 0;
				int bits = 16;
				for (uint8_t i = 0; i < 2; i++) {
					bits -= 8;
					payloadlen += (size_t)bytes[ptr++] << bits;
					remaining--;
				}
			}
			else if (payloadlen == 127) {
				if (remaining < 8) {
					return false;
				}
				payloadlen = 0;
				int bits = 64;
				for (uint8_t i = 0; i < 8; i++) {
					bits -= 8;
					payloadlen += (size_t)bytes[ptr++] << bits;
					remaining--;
				}
			}
			if (masking) {
				if (remaining < 4) {
					return false;
				}
				for (uint8_t i = 0; i < 4; i++) {
					masking_key[i] = bytes[ptr++];
					remaining--;
				}
			}
			if (remaining < payloadlen) {
				return false;
			}
			if (masking) {
				payload = unmask(masking_key, &bytes[ptr], payloadlen);
			}
			else {
				payload.assign((char *)&bytes[ptr], payloadlen);
			}
			remaining -= payloadlen;
			ptr += payloadlen;
			bytes_consumed = ptr;
			return true;
		};

		std::string CWebsocketFrame::Payload() {
			return payload;
		};

		bool CWebsocketFrame::isFinal() {
			return fin;
		};

		size_t CWebsocketFrame::Consumed() {
			return bytes_consumed;
		};

		opcodes CWebsocketFrame::Opcode() {
			return opcode;
		};

		CWebsocket::CWebsocket(boost::function<void(const std::string &packet_data)> _MyWrite, cWebem *_webEm, boost::function<void(const std::string &packet_data)> _WSWrite) :
			OUR_PING_ID("fd")
		{
			start_new_packet = true;
			MyWrite = _MyWrite;
			WsWrite = _WSWrite;

			// TODO: Don't hardcode this
			this->sUser = "Admin";
		}

		CWebsocket::~CWebsocket()
		{
		}

		boost::tribool CWebsocket::parse(const uint8_t *begin, size_t size, size_t &bytes_consumed, bool &keep_alive)
		{
			CWebsocketFrame frame;
			if (!frame.Parse(begin, size)) {
				bytes_consumed = 0;
				return boost::indeterminate;
			}
			bytes_consumed = frame.Consumed();
			if (start_new_packet) {
				packet_data.clear();
				last_opcode = frame.Opcode();
			}
			packet_data += frame.Payload();
			if (frame.isFinal()) {
				// packet is ready for packet handler
				start_new_packet = true;
				switch (last_opcode) {
				case opcode_continuation:
					// shouldn't occur here
					return false;
					break;
				case opcode_text:
					OnReceiveText(packet_data);
					return true;
					break;
				case opcode_binary:
					OnReceiveBinary(packet_data);
					return true;
					break;
				case opcode_close:
					SendClose("");
					keep_alive = false;
					return false;
					break;
				case opcode_ping:
					SendPong(packet_data);
					return false;
					break;
				case opcode_pong:
					OnPong(packet_data);
					return false;
					break;
				}
			}
			// packet waits for more fragments
			start_new_packet = false;
			return false;
		}

		// we receive a json request here. the final format is still to be decided
		// todo: move the body of this function to the websocket handler, so it can be
		// re-used from the proxy client
		// note: We mimic a web request here. This is just for testing purposes to see
		//       if everything works. We need a proper implementation here.
		void CWebsocket::OnReceiveText(const std::string &packet_data)
		{
			boost::tribool result = Handle(packet_data, false);
		}

		void CWebsocket::OnReceiveBinary(const std::string &packet_data)
		{
			// we assume we received a gzipped json request
			// todo: unzip the data
			std::string the_data = packet_data;
			OnReceiveText(the_data);
		}

		void CWebsocket::SendPing()
		{
			// todo: set the ping timer
			std::string frame = CWebsocketFrame::Create(opcode_ping, OUR_PING_ID, false);
			MyWrite(frame);
		}

		void CWebsocket::OnPong(const std::string &packet_data)
		{
			if (packet_data == OUR_PING_ID) {
				// todo: this was a response to one of our pings. reset the ping timer.
			}
		}

		void CWebsocket::SendPong(const std::string &packet_data)
		{
			std::string frame = CWebsocketFrame::Create(opcode_pong, packet_data, false);
			MyWrite(frame);
		}

		void CWebsocket::SendClose(const std::string &packet_data)
		{
			std::string frame = CWebsocketFrame::Create(opcode_close, packet_data, false);
			MyWrite(frame);
		}

		void CWebsocket::Start()
		{
			//handler.Start();
			m_Subscriber = m_mainworker.m_UpdateManager.Publisher.connect(boost::bind(&CWebsocket::DatabaseUpdate, this, _1));
		}

		void CWebsocket::Stop()
		{
			//handler.Stop();
			m_Subscriber.disconnect();
		}

		void CWebsocket::DatabaseUpdate(CUpdateEntry* pEntry)
		{
			try
			{
				Json::Value root;
				root["Count"] = 1;
				std::string		sTable = pEntry->m_Table + "s";
				for (unsigned int i = 0; i < vSubscriptions.size(); i++)
				{
					if (vSubscriptions[i] == sTable)
					{
						if (pEntry->m_Columns.size())
						{
							for (unsigned int i = 0; i < pEntry->m_Columns.size(); i++)
							{

								// Forbidden fields need to be stripped, just like for REST calls

								// TODO

								if (pEntry->m_ColumnTypes[i] == "INTEGER")
								{
									root[sTable.c_str()][0][pEntry->m_Columns[i]] = atoi(pEntry->m_Values[i].c_str());
								}
								else
								{
									root[sTable.c_str()][0][pEntry->m_Columns[i]] = pEntry->m_Values[i];
								}
							}
						}
						else
						{
							// For delete actions set the primary key to a negative value to signal front end
							root[sTable.c_str()][0][pEntry->m_Table+"ID"] = pEntry->m_RowIdx * -1;
						}

						// Add to the response
						Json::FastWriter	jWriter;
						std::string response = jWriter.write(root);
						_log.Log(LOG_NORM, "CWebsocket::%s Called, Message: %s", __func__, response.c_str());
						WsWrite(response);
						break;
					}
				}
			}
			catch (std::exception& e)
			{
				_log.Log(LOG_ERROR, "CWebsocket::%s Exception: %s", __func__, e.what());
			}
		}

		boost::tribool CWebsocket::Handle(const std::string& packet_data, bool outbound)
		{
			// Nope
			if (outbound)
			{
				_log.Log(LOG_ERROR, "CWebsocket::%s Error: Outbound should not be true.", __func__);
				return true;
			}


			Json::Reader reader;
			Json::Value value;
			if (!reader.parse(packet_data, value)) {
				return true;
			}

			//
			//	Valid inbound messages are : subscribe & unsubscribe
			//
			std::string sUnsubscribe = value["unsubscribe"].asString();
			if (sUnsubscribe.length())
			{
				for (unsigned int i = 0; i < vSubscriptions.size(); i++)
				{
					if (vSubscriptions[i] == sUnsubscribe)
					{
						vSubscriptions.erase(vSubscriptions.begin()+i);
						_log.Log(LOG_NORM, "CWebsocket::%s Processed unsubscribe for: %s.", __func__, sUnsubscribe.c_str());
						break;
					}
				}
			}

			std::string sSubscribe = value["subscribe"].asString();
			if (sSubscribe.length())
			{
				//	Note: For subscribe, the user must have GET access to the table being subscribed
				std::vector<std::vector<std::string> > result;
				result = m_sql.safe_query("SELECT CanGET  FROM TableAccess T, User U WHERE U.Username = '%q' AND U.RoleID = T.RoleID AND T.Name = '%q'", this->sUser.c_str(), sSubscribe.substr(0,sSubscribe.size()-1).c_str());
				if (!result.empty() && (result[0][0] == "1"))
				{
					vSubscriptions.push_back(sSubscribe);
					_log.Log(LOG_NORM, "CWebsocket::%s Processed subscription to: %s.", __func__, sSubscribe.c_str());
				}
			}

			return true;
		}
	}
}
