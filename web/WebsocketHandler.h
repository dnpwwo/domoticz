#pragma once

#include <boost/signals2.hpp>
#include "webserver/request.hpp"
#include "webserver/reply.hpp"
#include <boost/logic/tribool.hpp>
#include "../main/UpdatePublisher.h"
#include "../main/StoppableTask.h"
#include <thread>
#include <mutex>
#include <memory>

namespace http {
	namespace server {

		class cWebem;

		class CWebsocketHandler : public StoppableTask 
		{
		public:
			CWebsocketHandler(cWebem *pWebem, boost::function<void(const std::string &packet_data)> _MyWrite);
			~CWebsocketHandler();
			virtual boost::tribool Handle(const std::string &packet_data, bool outbound);
			virtual void Start();
			virtual void Stop();
			virtual void DatabaseUpdate(CUpdateEntry*);
		protected:
//			boost::function<void(const std::string &packet_data)> MyWrite;
			std::string sessionid;
			cWebem* myWebem;
			boost::signals2::connection m_Subscriber;
		private:
			std::shared_ptr<std::thread> m_thread;
			std::mutex m_mutex;
			void Do_Work();
		};

	}
}
