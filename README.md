# Creation a high-load server
***
* Download and install the vspkg library
* Build the project in VS2019
* Go to the site http://localhost:9001/hello
* In the developer tools write the code to initialize the user</br>
Example:
~~~

ws = new WebSocket("ws://localhost:9001/"); ws.onmessage = ({data}) => console.log("FROM SERVER: ", data);
ws.send(JSON.stringify({"command": "set_name", "name": "Петрович"}))
~~~


