/*
  UDPSendReceive.pde:
  This sketch receives UDP message strings, prints them to the serial port
  and sends an "acknowledge" string back to the sender

  A Processing sketch is included at the end of file that can be used to send
  and received messages for testing with a computer.

  created 21 Aug 2010
  by Michael Margolis

  This code is in the public domain.

  adapted from Ethernet library examples
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

#ifndef STASSID
#define STASSID "HiWiFi_62A784" //SSID
#define STAPSK "11112222"       //SSID-PASSWORD
#endif

unsigned int localPort = 8888; // local port to listen on

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged\r\n";   // a string to send back

WiFiUDP Udp;

void setup()
{
    Serial.begin(9600);          // 串口波特率设置
    WiFi.mode(WIFI_STA);         // WiFi工作模式
    WiFi.begin(STASSID, STAPSK); // 连接网络
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print('.');
        delay(500);
    }
    Serial.print("Connected! IP address: ");             // 连接完成提示
    Serial.println(WiFi.localIP());                      // 获取并打印IP
    Serial.printf("UDP server on port %d\n", localPort); // 本地udp 监听端口
    Udp.begin(localPort);                                // 开始udp 通信
    pinMode(D0, OUTPUT);                                 // set onboard LED as output
}

void loop()
{
    /*//发送数据
    Udp.beginPacket("192.168.199.56", 8888);
    Udp.write("helloworld");
    Serial.println("have send helloworld");
    Udp.endPacket();
    delay(1000);*/

    int packetSize = Udp.parsePacket();
    int d0_status = 0;
    //Serial.println(packetSize);
    if (packetSize)
    {
        Serial.print("接收到包大小：");
        Serial.println(packetSize);
        Serial.print("发送方：");
        IPAddress remote = Udp.remoteIP();
        for (int i = 0; i < 4; i++) // 循环输出远程端格式化IP地址
        {
            Serial.print(remote[i], DEC);
            if (i < 3)
            {
                Serial.print(".");
            }
        }
        Serial.print(",端口：");
        Serial.println(Udp.remotePort());               // 输出远程端端口
        Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE); // 接收数据
        Serial.println("内容:");
        Serial.println(packetBuffer); // 输出内容
        // d0_status = digitalRead(D0);
        // Serial.println("D0 状态为:");
        // Serial.println(status);
        if (packetBuffer)
        {
            digitalWrite(D0, 1);
            d0_status = digitalRead(D0);
            Serial.println("D0 状态已设置为为:");
            Serial.println(d0_status);
            delay(2000);
            digitalWrite(D0, 0);
        }
    }
    d0_status = digitalRead(D0);
    Serial.println("D0 现在的状态状态为:");
    Serial.println(d0_status);
    delay(1000);
}

/*
  test (shell/netcat):
  --------------------
	  nc -u 192.168.esp.address 8888
*/
