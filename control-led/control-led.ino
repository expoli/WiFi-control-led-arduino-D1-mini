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
#define STASSID "Xiaojia" //SSID
#define STAPSK "mmhjd4321"       //SSID-PASSWORD
#define CORE_IP "255.255.255.255"
#define MAX_DELAY_TIME 5000
#endif

unsigned int localPort = 8886; // local port to listen on
unsigned int remote_test_Port = 8888;
unsigned int wait_time = 0;
// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char ReplyBuffer[] = "acknowledged\r\n";   // a string to send back

WiFiUDP Udp;

void setup()
{
    Serial.begin(9600);          // 串口波特率设置
    WiFi.mode(WIFI_STA);         // WiFi工作模式
    WiFi.begin(STASSID, STAPSK); // 连接网络
    while (WiFi.status() != WL_CONNECTED&&wait_time<=10)
    {
        Serial.print('.');
        delay(500);
        wait_time++;
    }
    wait_time = 0;
    Serial.print("Connected! IP address: ");             // 连接完成提示
    Serial.println(WiFi.localIP());                      // 获取并打印IP
    Serial.printf("UDP server on port %d\n", localPort); // 本地udp 监听端口
    Udp.begin(localPort);                                // 开始udp 通信
    pinMode(D6, OUTPUT);                                 // set onboard LED as output
}

// 阀->核心请求连接	“xiaojiazhikong,3,[$DEVICE_NAME],CORE,0,0,1,end”
long workstatus = 2500;

void loop()
{
    // 发送数据
    // 阀->核心请求连接
    // 阀->核心请求连接	“xiaojiazhikong,3,[$DEVICE_NAME],CORE,0,0,1,end”
    char charBuf[100];
    char ipBuf[100];
    int remote_port = 0;

    String payload = "xiaojiazhikong,3,IOT,CORE,0,0,1,end";
    String StrStatusBuf;

    String ip_remote;
    int D6_status = 0;  // D6 状态值
    int received_sign = 0;  // 接收标志

    int packetSize = Udp.parsePacket();

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
        // 循环输出远程端格式化IP地址
        for (int i = 0; i < 4; i++)
        {
            ip_remote += remote[i];
            if (i<3)
            {
                ip_remote += ".";
            }
        }
        // 
        ip_remote.toCharArray(charBuf,100);
        Serial.print(",端口：");
        Serial.println(Udp.remotePort());               // 输出远程端端口

        int remote_port = Udp.remotePort();
        
        Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE); // 接收数据
        Serial.print("内容:");
        Serial.println(packetBuffer); // 输出内容

        String StrPacketBuffer = packetBuffer;
        if (StrPacketBuffer.startsWith("xiaojiazhikong,"))
        {
            Serial.println("接收到控制信号");
            received_sign = 1;
        }

    }

    if (received_sign)
    {
        received_sign = 0;
        String StrPacketBuffer = packetBuffer;

        int num = StrPacketBuffer.startsWith("xiaojiazhikong,2,CORE,IOT,5,0,1,");

        Serial.print("字符串匹配结果：");
        Serial.println(num);
        // 核心->阀回应连接	“xiaojiazhikong,2,[$DEVICE_NAME], [$DEVICE_NAME2],1,0,1,end”
        if (StrPacketBuffer.startsWith("xiaojiazhikong,2,CORE,IOT,1,0,1,"))
        {
            Serial.println("已和核心链接..............");

            Serial.print("数据包长度：");
            Serial.println(StrPacketBuffer.length());
            StrPacketBuffer.~String();
        }
        // 核心->阀请求工作状态	“xiaojiazhikong,2,[$DEVICE_NAME], [$DEVICE_NAME2],5,0,1,end”
        else if (StrPacketBuffer.startsWith("xiaojiazhikong,2,CORE,IOT,5,0,1,"))
        {
            StrStatusBuf += "xiaojiazhikong,3,IOT,CORE,6,1,";
            StrStatusBuf += workstatus;
            StrStatusBuf += ",1,end";
            StrStatusBuf.toCharArray(charBuf,100);
            Udp.beginPacket(CORE_IP, remote_test_Port);
            Udp.write(charBuf);
            Udp.endPacket();

            Serial.print("-> CORE 工作状态已返回");
            Serial.println(workstatus>>1);
            
            Serial.print("数据包长度：");
            Serial.println(StrPacketBuffer.length());
            StrPacketBuffer.~String();
        }
        // 核心->阀设置工作状态	“xiaojiazhikong,2,[$DEVICE_NAME], [$DEVICE_NAME2],7,1,[$DEVICE_STATE],1,end”
        else if (StrPacketBuffer.startsWith("xiaojiazhikong,2,CORE,IOT,7,1,"))
        {
            StrStatusBuf = StrPacketBuffer.substring(30,34);
            workstatus = StrStatusBuf.toInt();
            StrStatusBuf = "xiaojiazhikong,3,IOT,CORE,6,1,";
            StrStatusBuf += workstatus;
            StrStatusBuf += ",1,end";
            StrStatusBuf.toCharArray(charBuf,100);
            Udp.beginPacket(CORE_IP, remote_test_Port);
            Udp.write(charBuf);
            Udp.endPacket();

            Serial.print("-> CORE 工作状态已设置为");
            Serial.println(charBuf);

            Serial.print("数据包长度：");
            Serial.println(StrPacketBuffer.length());
            StrPacketBuffer.~String();
        }
    }
    digitalWrite(D6, 1);        // D1 置1
    D6_status = digitalRead(D6);    // 读取D6 引脚状态
    // Serial.println("EX:D6 状态已设置为为:");
    // Serial.println(D6_status);      // 打印现在的状态
    delay(MAX_DELAY_TIME-workstatus);           // 继电器通电两秒

    digitalWrite(D6, 0);            // D6 端口置0
    D6_status = digitalRead(D6);    // 读取D6 引脚状态
    // Serial.println("EX:D6 状态已设置为为:");
    // Serial.println(D6_status);      // 打印现在的状态
    delay(workstatus);
}

/*
  test (shell/netcat):
  --------------------
	  nc -u 192.168.esp.address 8888
*/
