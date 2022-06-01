#include <iostream>
#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>
#include <string>

using namespace std;

using json = nlohmann::json;

//Структура данных, привязанная к каждому соединению
struct UserData {
	int user_id;
	std::string name;
};

using UWEBSOCK = uWS::WebSocket<false, true, UserData>;

const std::string COMMAND = "command";
const std::string PRIVATE_MSG = "PRIVATE_MSG";
const std::string PUBLIC_MSG = "PUBLIC_MSG";
const std::string USER_ID_TO = "user_id_to";
const std::string USER_ID_FROM = "user_id_from";
const std::string TEXT = "text";

void processMessage(UWEBSOCK* ws, std::string_view message) {
	json data = json::parse(message);
	UserData* userData = ws->getUserData();

	if (data[COMMAND] == PRIVATE_MSG)
	{
		//принимаем входные данные
		int user_id = data[USER_ID_TO];
		std::string text = data[TEXT];

		//готовим ответ
		json response;
		response[COMMAND] = PRIVATE_MSG;
		response[USER_ID_FROM] = userData->user_id;//user_id отправителя
		response[TEXT] = text;
		//получатель
		ws->publish("user" + std::to_string(user_id), response.dump());
		cout << "User №" << userData->user_id << " send message to user №" << user_id << std::endl;
	}
	else if (data[COMMAND] == PUBLIC_MSG)
	{
		//принимаем входные данные
		int user_id = data[USER_ID_TO];
		std::string text = data[TEXT];

		//готовим ответ
		json response;
		response[COMMAND] = PRIVATE_MSG;
		response[USER_ID_FROM] = userData->user_id;//user_id отправителя
		response[TEXT] = text;
		//получатель
		ws->publish("user" + std::to_string(user_id), response.dump());
		cout << "User №" << userData->user_id << " send message to user №" << user_id << std::endl;
	}

	//COMMAND
	//PUBLIC_MSG
	//SET_NAME
	//STATUS (omline/offline)
};

int main()
{
	int latest_user_id = 10;
	uWS::App()
		.get("/hello", [](auto* res, auto* req) {
			/* You can efficiently stream huge files too */
			res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("Hello HTTP!");
		}).ws<UserData>("/*", {
				//Just a few of the available handlers
				.open = [&latest_user_id](auto* ws) {
					UserData* data = ws->getUserData();
					data->user_id = latest_user_id++;

					std::cout << "User connected ID = " << data->user_id << std::endl;
					ws->subscribe("user" + std::to_string(data->user_id));
					//user10, user11, user12...
					//кто-то подключается к серверу
					//Выдать id
					//Зарегистрировать в каналах
					//Придумать протокол для сообщений, статусов (онлайн/оффлайн)
				},
				.message = [](auto* ws, std::string_view message, uWS::OpCode opCode) {
					//Кто-то отправляет на сервер данные (в нашем случае JSON)
					//Понять, что он хочет
					//Ответить
					processMessage(ws, message);
				}//,
				//.close = [](auto* ws, int code, std::string_view message) {
				//	//Кто-то отключается от сервера
				//}			
				}).listen(9001, [](auto* listenSocket) {

					if (listenSocket) {
						std::cout << "URL http://localhost:" << 9001 << std::endl;
					}

					}).run();
}

