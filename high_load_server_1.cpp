#include <iostream>
#include <uwebsockets/App.h>
#include <nlohmann/json.hpp>
#include <string>
#include <map>

using namespace std;

using json = nlohmann::json;

//Структура данных, привязанная к каждому соединению
struct UserData {
	int user_id;
	std::string name;
};

std::map<int, UserData*> all_users; //Все подключеннные пользоваетли

using UWEBSOCK = uWS::WebSocket<false, true, UserData>;

const std::string COMMAND = "command";

const std::string PUBLIC_CHANNEL = "public_channel";

const std::string PRIVATE_MSG = "private_msg";
const std::string PUBLIC_MSG = "public_msg";
const std::string STATUS = "status";//{command:STATUS, online:True/False, user_id: int, name: string}
const std::string SET_NAME = "set_name";

const std::string USER_ID = "user_id";
const std::string USER_ID_TO = "user_id_to";
const std::string USER_ID_FROM = "user_id_from";
const std::string TEXT = "text";
const std::string NAME = "name";
const std::string ONLINE = "online";

std::string status(UserData* data, bool online) {
	json response;
	response[COMMAND] = STATUS;
	response[USER_ID] = data->user_id;
	response[NAME] = data->name;
	response[STATUS] = online;
	return response.dump();
}

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
		cout << "User N" << userData->user_id << " send message to user №" << user_id << std::endl;
	}
	
	if (data[COMMAND] == PUBLIC_MSG)
	{
		json response;
		response[COMMAND] = PUBLIC_MSG;
		response[TEXT] = data[TEXT];
		response[USER_ID_FROM] = userData->user_id;
		ws->publish(PUBLIC_CHANNEL, response.dump());
		std::cout << "User N" << userData->user_id << " sends public message";
	}

	if (data[COMMAND] == SET_NAME)
	{
		std::string name = data[NAME];
		userData->name = name;
		ws->publish(PUBLIC_CHANNEL, status(userData, true));
		std::cout << "User N" << userData->user_id << " set their name\n";
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
		.ws<UserData>("/*", {
				//Just a few of the available handlers
				.open = [&latest_user_id](auto* ws) {
					UserData* data = ws->getUserData();
					data->user_id = latest_user_id++;
					data->name = "UNNAMED";

					std::cout << "User connected ID = " << data->user_id << std::endl;

					ws->subscribe("user" + std::to_string(data->user_id));
					ws->publish(PUBLIC_CHANNEL, status(data, true));
					ws->subscribe(PUBLIC_CHANNEL);
					
					for (const auto& entry : all_users)
					{
						ws->send(status(entry.second, true), uWS::OpCode::TEXT);
					}


					all_users[data->user_id] = data;
				},
				.message = [](auto* ws, std::string_view message, uWS::OpCode opCode) {
					//Кто-то отправляет на сервер данные (в нашем случае JSON)
					//Понять, что он хочет
					//Ответить
					processMessage(ws, message);
				},
				.close = [](auto* ws, int code, std::string_view message) {
					//Кто-то отключается от сервера
					UserData* data = ws->getUserData();
					ws->publish(PUBLIC_CHANNEL, status(data, false));//OFFLINE
					std::cout << "User N" << data->user_id << " disconnected\n";
					all_users.erase(data->user_id);
				}			
				}).listen(9001, [](auto* listenSocket) {

					if (listenSocket) {
						std::cout << "URL http://localhost:" << 9001 << std::endl;
					}

					}).run();
}

