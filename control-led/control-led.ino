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
#include <string.h>
#include <strings.h>

// 阀->核心请求连接	“xiaojiazhikong,3,[$DEVICE_NAME],CORE,0,0,1,end”
// 核心->阀回应连接	“xiaojiazhikong,2,[$DEVICE_NAME], [$DEVICE_NAME2],1,0,1,end”
// 核心->阀请求工作状态	“xiaojiazhikong,2,[$DEVICE_NAME], [$DEVICE_NAME2],5,0,1,end”
// 阀->核心回应工作状态	“xiaojiazhikong,3,[$DEVICE_NAME], [$DEVICE_NAME2],6,1,[$DEVICE_STATE],1,end”
// 核心->阀设置工作状态	“xiaojiazhikong,2,[$DEVICE_NAME], [$DEVICE_NAME2],7,1,[$DEVICE_STATE],1,end”

#ifndef STASSID
#define STASSID "shuangzu" //SSID
#define STAPSK "mmhjd4321"       //SSID-PASSWORD
#define CORE_IP "192.168.199.56"
#define IOT_LINK_CORE "xiaojiazhikong,3,[$DEVICE_NAME],CORE,0,0,1,end"
#define CORE_RELAY_IOT "xiaojiazhikong,2,[$DEVICE_NAME],[$DEVICE_NAME2],1,0,1,end"
#define CORE_REQUEST_WORK_STATUS "xiaojiazhikong,2,[$DEVICE_NAME],[$DEVICE_NAME2],5,0,1,end" 
#define IOT_RELAY_STATUS "xiaojiazhikong,3,[$DEVICE_NAME],[$DEVICE_NAME2],6,1,[$DEVICE_STATE],1,end"
#define CORE_SET_STATUS "xiaojiazhikong,2,[$DEVICE_NAME],[$DEVICE_NAME2],7,1,[$DEVICE_STATE],1,end"
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
    pinMode(D6, OUTPUT);                                 // set onboard LED as output
}

// 阀->核心请求连接	“xiaojiazhikong,3,[$DEVICE_NAME],CORE,0,0,1,end”
// 核心->阀回应连接	“xiaojiazhikong,2,[$DEVICE_NAME], [$DEVICE_NAME2],1,0,1,end”
// 核心->阀请求工作状态	“xiaojiazhikong,2,[$DEVICE_NAME], [$DEVICE_NAME2],5,0,1,end”
// 阀->核心回应工作状态	“xiaojiazhikong,3,[$DEVICE_NAME], [$DEVICE_NAME2],6,1,[$DEVICE_STATE],1,end”
// 核心->阀设置工作状态	“xiaojiazhikong,2,[$DEVICE_NAME], [$DEVICE_NAME2],7,1,[$DEVICE_STATE],1,end”
long workstatus = 2000;

void loop()
{
    // 发送数据
    // 阀->核心请求连接
    // 阀->核心请求连接	“xiaojiazhikong,3,[$DEVICE_NAME],CORE,0,0,1,end”
    char charBuf[100];
    char ipBuf[100];
    int remote_port = 0;
    String payload = IOT_LINK_CORE;
    String StrStatusBuf;

    payload.replace("[$DEVICE_NAME]","IOT");
    payload.toCharArray(charBuf,100);
    Udp.beginPacket(CORE_IP, 8888);
    Udp.write(charBuf);
    Serial.println(charBuf);
    Udp.endPacket();
    // delay(1000);

    int packetSize = Udp.parsePacket();
    int D6_status = 0;

    int received_sign = 0;
    //Serial.println(packetSize);
    if (packetSize)
    {
        Serial.print("接收到包大小：");
        Serial.println(packetSize);
        Serial.print("发送方：");
        IPAddress remote = Udp.remoteIP();
        String ip_remote;
        for (int i = 0; i < 4; i++) // 循环输出远程端格式化IP地址
        {
            Serial.print(remote[i], DEC);
            if (i < 3)
            {
                Serial.print(".");
            }
        }
        for (int i = 0; i < 4; i++)
        {
            ip_remote += remote[i];
            if (i<3)
            {
                ip_remote += ".";
            }
        }

        ip_remote.toCharArray(ipBuf,100);
        Serial.print(",端口：");
        Serial.println(Udp.remotePort());               // 输出远程端端口

        int remote_port = Udp.remotePort();
        
        Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE); // 接收数据
        Serial.println("内容:");
        Serial.println(packetBuffer); // 输出内容

        String StrPacketBuffer = packetBuffer;
        if (StrPacketBuffer.startsWith("xiaojiazhikong,"))
            received_sign = 1;
    }

    if (received_sign)
    {
        String StrPacketBuffer = packetBuffer;
        payload = "xiaojiazhikong,2,CORE,IOT,1,0,1,end";
        // payload.replace("[$DEVICE_NAME]","CORE");
        // payload.replace("[$DEVICE_NAME2]","IOT");
        // 核心->阀回应连接	“xiaojiazhikong,2,[$DEVICE_NAME], [$DEVICE_NAME2],1,0,1,end”
        if (StrPacketBuffer.compareTo("xiaojiazhikong,2,CORE,IOT,1,0,1,end")>=8)
        {
            Serial.println(StrPacketBuffer.compareTo("xiaojiazhikong,2,CORE,IOT,1,0,1,end"));
            Serial.println("已和核心链接..............");
        }
        // 核心->阀请求工作状态	“xiaojiazhikong,2,[$DEVICE_NAME], [$DEVICE_NAME2],5,0,1,end”
        else if (StrPacketBuffer.compareTo("xiaojiazhikong,2,CORE,IOT,5,0,1,end")>=8)
        {
            Serial.print("工作状态已返回:");

            StrStatusBuf += workstatus;
            StrStatusBuf.toCharArray(charBuf,100);
            Udp.beginPacket("192.168.1.121", 44820);
            Udp.write("工作状态已返回:");
            Udp.write(charBuf);
            Udp.endPacket();
            Serial.println(workstatus>>1);
        }
        // 核心->阀设置工作状态	“xiaojiazhikong,2,[$DEVICE_NAME], [$DEVICE_NAME2],7,1,[$DEVICE_STATE],1,end”
        else if (StrPacketBuffer.startsWith("xiaojiazhikong,2,CORE,IOT,7,1,"))
        {
            StrStatusBuf = StrPacketBuffer.substring(30,38);
            workstatus = StrStatusBuf.toInt();
            StrStatusBuf.toCharArray(charBuf,100);
            Udp.beginPacket("192.168.1.121", 44820);
            Udp.write("工作状态已设置为:");
            Udp.write(workstatus);
            Udp.endPacket();
            Serial.print("工作状态已设置为");
            Serial.println(charBuf);
        }
    }
    digitalWrite(D6, 1);        // D1 置1
    D6_status = digitalRead(D6);    // 读取D6 引脚状态
    Serial.println("D6 状态已设置为为:");
    Serial.println(D6_status);      // 打印现在的状态
    delay(3000-workstatus);           // 继电器通电两秒

    digitalWrite(D6, 0);            // D6 端口置0
    D6_status = digitalRead(D6);    // 读取D6 引脚状态
    Serial.println("D6 状态已设置为为:");
    Serial.println(D6_status);      // 打印现在的状态
    delay(workstatus);
}

/*
  test (shell/netcat):
  --------------------
	  nc -u 192.168.esp.address 8888
*/
