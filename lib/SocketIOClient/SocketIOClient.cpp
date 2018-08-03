/*
socket.io-arduino-client: a Socket.IO client for the Arduino
Based on the Kevin Rohling WebSocketClient & Bill Roy Socket.io Lbrary
Copyright 2015 Florent Vidal
Supports Socket.io v1.x
Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:
The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

/*
 * Modified by RoboJay
 */

#include <SocketIOClient.h>

SocketIOClient::SocketIOClient() {
	for (int i = 0; i < MAX_ON_HANDLERS; i++) {
		onId[i] = "";
	}
}

bool SocketIOClient::connect(String thehostname, int theport) {
	thehostname.toCharArray(hostname, MAX_HOSTNAME_LEN);
	port = theport;

	if (!internets.connect(hostname, theport)){ return false; }

	sendHandshake(hostname);
	return readHandshake();
}

bool SocketIOClient::connectHTTP(String thehostname, int theport) {
	thehostname.toCharArray(hostname, MAX_HOSTNAME_LEN);
	port =  theport;
	if (!internets.connect(hostname, theport)){ return false; }
	return true;
}

bool SocketIOClient::reconnect(String thehostname, int theport) {
	thehostname.toCharArray(hostname, MAX_HOSTNAME_LEN);
	port = theport;

	if (!internets.connect(hostname, theport)){ return false; }

	sendHandshake(hostname);
	return readHandshake();
}

bool SocketIOClient::connected() { return internets.connected();}

void SocketIOClient::disconnect() { internets.stop(); }

void SocketIOClient::eventHandler(int index) {
	String id = "";
	String data = "";
	String rcvdmsg = "";
	int sizemsg = databuffer[index + 1]; // 0-125 byte, index ok Fix provide by Galilei11. Thanks
	if (databuffer[index + 1]>125) {
		sizemsg = databuffer[index + 2];   // 126-255 byte
		index += 1;                        // index correction to start
	}
	if (ioDebug) Serial.print("[eventHandler] Message size = "); //Can be used for debugging
	if (ioDebug) Serial.println(sizemsg);			             //Can be used for debugging
	for (int i = index + 2; i < index + sizemsg + 2; i++) { rcvdmsg += (char)databuffer[i]; }
	if (ioDebug) Serial.print("[eventHandler] Received message = "); //Can be used for debugging
	if (ioDebug) Serial.println(rcvdmsg);				               //Can be used for debugging
	switch (rcvdmsg[0]) {
	  case '2':
		  if (ioDebug) Serial.println("[eventHandler] Ping received - Sending Pong");
		  heartbeat(1);
		  break;
	  case '3':
		  if (ioDebug) Serial.println("[eventHandler] Pong received - All good");
		  break;
	  case '4':
		  switch (rcvdmsg[1]) {
		    case '0':
			    if (ioDebug) Serial.println("[eventHandler] Upgrade to WebSocket confirmed");
			    break;
		    case '2':
			    id = rcvdmsg.substring(4, rcvdmsg.indexOf("\","));
          if (ioDebug){ Serial.println("[eventHandler] id = " + id); }
          data = rcvdmsg.substring(rcvdmsg.indexOf("\",") + 3, rcvdmsg.length () - 2);
			    if (ioDebug){ Serial.println("[eventHandler] data = " + data); }
			    for (uint8_t i = 0; i < onIndex; i++) {
				    if (id == onId[i]) {
					    if (ioDebug) Serial.println("[eventHandler] Found handler = " + String(i));
					    (*onFunction[i])(data);
				    }
			    }
			    break;
		  }
	  }
}

bool SocketIOClient::monitor() {
	int index = -1;
	int index2 = -1;
	String tmp = "";
	*databuffer = 0;
	static unsigned long pingTimer = 0;

	if (!internets.connected()) {
		if (ioDebug) Serial.println("[monitor] Client not connected.");
		if (connect(hostname, port)) {
			if (ioDebug) Serial.println("[monitor] Connected.");
			return true;
		} else {
			if (ioDebug) Serial.println("[monitor] Can't connect. Aborting.");
		}
	}

	// the PING_INTERVAL from the negotiation phase should be used
	// this is a temporary hack
	if (internets.connected() && millis() >= pingTimer) {
		heartbeat(0);
		pingTimer = millis() + PING_INTERVAL;
	}

	if (!internets.available()){ return false;}
	while (internets.available()) { // read availible internets
		readLine();
		tmp = databuffer;
		dataptr = databuffer;
		index = tmp.indexOf((char)129);	//129 DEC = 0x81 HEX = sent for proper communication
		index2 = tmp.indexOf((char)129, index + 1);
		if (index != -1)  { eventHandler(index); }
		if (index2 != -1) { eventHandler(index2);}
	}
  return false;
}

void SocketIOClient::sendHandshake(char hostname[]) {
	internets.println(F("GET /socket.io/1/?transport=polling&b64=true HTTP/1.1"));
	internets.print(F("Host: "));
	internets.println(hostname);
	internets.println(F("Origin: Arduino\r\n"));
}

bool SocketIOClient::waitForInput(void) {
	unsigned long now = millis();
	while (!internets.available() && ((millis() - now) < 30000UL)) { ; }
	return internets.available();
}

void SocketIOClient::eatHeader(void) {
	while (internets.available()) {	// consume the header
		readLine();
		if (strlen(databuffer) == 0) { break; }
	}
}

bool SocketIOClient::readHandshake() {
	if (!waitForInput()){ return false; }
	// check for happy "HTTP/1.1 200" response
	readLine();
	if (atoi(&databuffer[9]) != 200) {
		while (internets.available()) readLine();
		internets.stop();
		return false;
	}
	eatHeader();
	readLine();
	String tmp = databuffer;

	int sidindex = tmp.indexOf("sid");
	int sidendindex = tmp.indexOf("\"", sidindex + 6);
	int count = sidendindex - sidindex - 6;

	for (int i = 0; i < count; i++) {
		sid[i] = databuffer[i + sidindex + 6];
	}
	if (ioDebug) Serial.print(F("[readHandshake] Connected. SID="));
	if (ioDebug) Serial.println(sid);	// sid:transport:timeout

	while (internets.available()) readLine();
	internets.stop();
	delay(1000);

	// reconnect on websocket connection
	if (!internets.connect(hostname, port)) {
		if (ioDebug) Serial.print(F("[readHandshake] Websocket failed."));
		return false;
	}
	if (ioDebug) Serial.println(F("[readHandshake] Connecting via Websocket"));

	internets.print(F("GET /socket.io/1/websocket/?transport=websocket&b64=true&sid="));
	internets.print(sid);
	internets.print(F(" HTTP/1.1\r\n"));
	internets.print(F("Host: "));
	internets.print(hostname);
	internets.print("\r\n");
	internets.print(F("Origin: ArduinoSocketIOClient\r\n"));
	internets.print(F("Sec-WebSocket-Key: "));
	internets.print(sid);
	internets.print("\r\n");
	internets.print(F("Sec-WebSocket-Version: 13\r\n"));
	internets.print(F("Upgrade: websocket\r\n"));	// must be camelcase ?!
	internets.println(F("Connection: Upgrade\r\n"));

	if (!waitForInput()) { return false; }
	readLine();
	if (atoi(&databuffer[9]) != 101) {	// check for "HTTP/1.1 101 response, means Updrage to Websocket OK
		while (internets.available()) { readLine(); }
		internets.stop();
		return false;
	}
	readLine();
	readLine();
	readLine();
	for (int i = 0; i < 28; i++) {
		key[i] = databuffer[i + 22];	//key contains the Sec-WebSocket-Accept, could be used for verification
	}
	eatHeader();

	/*
	Generating a 32 bits mask requiered for newer version
	Client has to send "52" for the upgrade to websocket
	*/
	randomSeed(analogRead(0));
	String mask = "";
	String masked = "52";
	String message = "52";
	for (int i = 0; i < 4; i++) {	//generate a random mask, 4 bytes, ASCII 0 to 9
		char a = random(48, 57);
		mask += a;
	}
	for (int i = 0; i < message.length(); i++){
		masked[i] = message[i] ^ mask[i % 4];	//apply the "mask" to the message ("52")
  }

	internets.print((char)0x81);	//has to be sent for proper communication
	internets.print((char)130);	//size of the message (2) + 128 because message has to be masked
	internets.print(mask);
	internets.print(masked);

	monitor();		// treat the response as input
	return true;
}

void SocketIOClient::getREST(String path) {
	String message = "GET /" + path + "/ HTTP/1.1";
	internets.println(message);
	internets.print(F("Host: ")); internets.println(hostname);
	internets.println(F("Origin: Arduino"));
	internets.println(F("Connection: close\r\n"));
}

void SocketIOClient::postREST(String path, String type, String data) {
	String message = "POST /" + path + "/ HTTP/1.1";
	internets.println(message);
	internets.print(F("Host: "));           internets.println(hostname);
	internets.println(F("Origin: Arduino"));
	internets.println(F("Connection: close\r\n"));
	internets.print(F("Content-Length: ")); internets.println(data.length());
	internets.print(F("Content-Type: "));   internets.println(type);
	internets.println("\r\n");
	internets.println(data);

}

void SocketIOClient::putREST(String path, String type, String data) {
	String message = "PUT /" + path + "/ HTTP/1.1";
	internets.println(message);
	internets.print(F("Host: "));           internets.println(hostname);
	internets.println(F("Origin: Arduino"));
	internets.println(F("Connection: close\r\n"));
	internets.print(F("Content-Length: ")); internets.println(data.length());
	internets.print(F("Content-Type: "));   internets.println(type);
	internets.println("\r\n");
	internets.println(data);
}

void SocketIOClient::deleteREST(String path) {
	String message = "DELETE /" + path + "/ HTTP/1.1";
	internets.println(message);
	internets.print(F("Host: ")); internets.println(hostname);
	internets.println(F("Origin: Arduino"));
	internets.println(F("Connection: close\r\n"));
}

void SocketIOClient::readLine() {
	for (int i = 0; i < DATA_BUFFER_LEN; i++) { databuffer[i] = ' '; }
	dataptr = databuffer;
	while (internets.available() && (dataptr < &databuffer[DATA_BUFFER_LEN - 2])){
		char c = internets.read();
		if (c == 0) {if (ioDebug) Serial.print("");}
		else if (c == 255) {if (ioDebug) Serial.println("");}
		else if (c == '\r') { ; }
		else if (c == '\n') break;
		else *dataptr++ = c;
	}
	*dataptr = 0;
}


void SocketIOClient::emit(String id, String data) {
	String message = "42[\"" + id + "\"," + data + "]";
	if (ioDebug) Serial.println("[emit] " + message);
	int header[10];
	header[0] = 0x81;
	int msglength = message.length();
	if (ioDebug) Serial.println("[emit] " + String(msglength));
	randomSeed(analogRead(0));
	String mask = "";
	String masked = message;
	for (int i = 0; i < 4; i++) {
		char a = random(48, 57);
		mask += a;
	}
	for (int i = 0; i < msglength; i++){
		masked[i] = message[i] ^ mask[i % 4];
	}

	internets.print((char)header[0]);	// has to be sent for proper communication
									                  // Depending on the size of the message
	if (msglength <= 125) {
		header[1] = msglength + 128;
		internets.print((char)header[1]);	//size of the message + 128 because message has to be masked
	} else if (msglength >= 126 && msglength <= 65535) {
		header[1] = 126 + 128;
		internets.print((char)header[1]);
		header[2] = (msglength >> 8) & 255;
		internets.print((char)header[2]);
		header[3] = (msglength)& 255;
		internets.print((char)header[3]);
	} else {
		header[1] = 127 + 128;
		internets.print((char)header[1]);
		header[2] = (msglength >> 56) & 255;
		internets.print((char)header[2]);
		header[3] = (msglength >> 48) & 255;
		internets.print((char)header[4]);
		header[4] = (msglength >> 40) & 255;
		internets.print((char)header[4]);
		header[5] = (msglength >> 32) & 255;
		internets.print((char)header[5]);
		header[6] = (msglength >> 24) & 255;
		internets.print((char)header[6]);
		header[7] = (msglength >> 16) & 255;
		internets.print((char)header[7]);
		header[8] = (msglength >> 8) & 255;
		internets.print((char)header[8]);
		header[9] = (msglength)& 255;
		internets.print((char)header[9]);
	}
	internets.print(mask);
	internets.print(masked);
}

void SocketIOClient::heartbeat(int select) {
	randomSeed(analogRead(0));
	String mask = "";
	String masked = "";
	String message = "";
	if (select == 0) {
		masked = "2";
		message = "2";
	} else {
		masked = "3";
		message = "3";
	}
	for (int i = 0; i < 4; i++) { //generate a random mask, 4 bytes, ASCII 0 to 9
		char a = random(48, 57);
		mask += a;
	}
	for (int i = 0; i < message.length(); i++){
		masked[i] = message[i] ^ mask[i % 4];	//apply the "mask" to the message ("2" : ping or "3" : pong)
  }
	internets.print((char)0x81);	//has to be sent for proper communication
	internets.print((char)129);	//size of the message (1) + 128 because message has to be masked
	internets.print(mask);
	internets.print(masked);
}

void SocketIOClient::on(String id, functionPointer function) {
	if (onIndex == MAX_ON_HANDLERS){ return; } // oops... to many...
	onFunction[onIndex] = function;
	onId[onIndex] = id;
	onIndex++;
}

