/*			                       *
 *  Guindaste  1.0.1                           *
 *  26 março 2019 - 02 abril 2019              *
 *  Laboratório de Projetos III - Grupo C      *
 *                                             */

#include <SPI.h>
#include <Ethernet.h>
//#include <EEPROM.h>
#include <EEPROMAnything.h>


//Versão
static int _versao[] = {1, 0, 0};

//Rede
static byte _mac[] = { 0x90, 0xA2, 0xDA, 0x00, 0x5D, 0x8B };

//Define
#define ledStatus 13
#define telnetBufferSize 10

struct configuracoes_t
{
  IPAddress ip;
  IPAddress subnet;
  IPAddress gateway;
  unsigned int porta;
  long ethernetTimeout;
  unsigned int posicao;
  unsigned int angulo;  
} _configuracoes;


//Servidor Telnet
EthernetServer Server(5000);
EthernetClient Client;

char _telnetBuffer[telnetBufferSize];
int _telnetNunChar = 0;
int _telnetVetorEntrada =0;

int _estadoLed = 0;
unsigned long _tempoVida = 0;

void setup() {  
  //ledStatus
  pinMode(ledStatus, OUTPUT); 
  
  //EEPROM
  //0.1 - Versão, Sub-Versão  
  if ((_versao[0] != EEPROM.read(0)) || (_versao[1] != EEPROM.read(1)))
  {
    //EEPROM Incompatível
    Set_EEPROM_Default();
  }
  else
  {
    //2.3.4.5.6.7 - MAC 
    //MAC
    //8 em diante
    //ip, subnet, gateway, porta, velocidade_serial1, velocidade_serial2
    EEPROM_readAnything(8, _configuracoes);
  }
  
  //Rede
  Ethernet.begin(_mac, _configuracoes.ip, _configuracoes.gateway, _configuracoes.subnet);
  
  //Telnet
  Server = EthernetServer(_configuracoes.porta);
  Server.begin();
  _tempoVida = millis();

}
void loop() 
{
  
  if(millis() - _tempoVida >= 1000)
  {
    digitalWrite(ledStatus, _estadoLed);
    _estadoLed = !_estadoLed;
    _tempoVida = millis();
  }
    
  
  //Recebe Comandos
  Client = Server.available();
  if(Client.available()) 
  {  
    Telnet_Recebe_Comando(); 
  }
  
  
  //ledStatus
  //_estado_led = !_estado_led;
  //digitalWrite(ledStatus, _estado_led);
  //_tempoVida = millis();    

}

//EEPROM
void Set_EEPROM_Default()
{
  
  //0, 1 - Versão, Sub-Versãp
  EEPROM.write(0, _versao[0]);
  EEPROM.write(1, _versao[1]);
  
  //2.3.4.5.6.7 - MAC 
  //MAC
  
  _configuracoes.ip = IPAddress(172, 16, 0, 201);
  _configuracoes.subnet = IPAddress(255, 255, 255, 0);
  _configuracoes.gateway = IPAddress(172, 16, 0, 3);
  _configuracoes.porta = 5000;
  _configuracoes.ethernetTimeout = 300000;
  _configuracoes.posicao = 0;
  _configuracoes.angulo = 0;
  
  EEPROM_writeAnything(8, _configuracoes);
  
}

void Telnet_Limpa_Buffer()
{
  for (int x = 0; x < telnetBufferSize; x++)
  {
    _telnetBuffer[x] = '\0';
  }
  _telnetNunChar = 0;
}

void Telnet_Recebe_Comando()
{ 
  char c;   
  _telnetVetorEntrada = Client.available();  
  do 
  {
    c = Client.read();
    if(c == 0x0d) 
    {
      Telnet_Trata_Comando(_telnetBuffer);
      Telnet_Limpa_Buffer();
    }
    else
    {
      _telnetBuffer[_telnetNunChar] = c;
      _telnetNunChar++;
      _telnetVetorEntrada--;
    }
  }
  while(_telnetNunChar <= telnetBufferSize && c != 0x0d && _telnetVetorEntrada > 0);  
  if(_telnetNunChar >= telnetBufferSize) 
  {  
    Telnet_Trata_Comando(_telnetBuffer);
    Telnet_Limpa_Buffer();
  }  
}

void Telnet_Trata_Comando(char* Texto)
{
  char* parametro;
  parametro = strtok(Texto, " ,");
  while( parametro != NULL)
  {
    Telnet_Executa_Comando(parametro);
    parametro = strtok(NULL, " ,");
  }
}

void Telnet_Executa_Comando(String Texto)
{
  if (Texto.length() >= 4)
  {
    for (int i = 0; i < sizeof(Texto) - 1; i++)
    {
      if (Texto[i] == '!')
      {  
        //DO
        if ((Texto[i + 1] == 'p') || (Texto[i + 1] == 'P'))
        {
          if ((Texto[i + 2] == 'o') || (Texto[i + 2] == 'O'))
          {
            if ((Texto[i + 3] == 's') || (Texto[i + 3] == 'S'))
            {
              Server.print("angulo = ");
              Server.print(_configuracoes.angulo);
              Server.print(" ");
              Server.print("posicao = ");
              Server.println(_configuracoes.posicao);
            }
          }
        }
        else if ((Texto[i + 1] == 'm') || (Texto[i + 1] == 'M'))
        {
          if ((Texto[i + 2] == 'o') || (Texto[i + 2] == 'O'))
          {
            if ((Texto[i + 3] == 'v') || (Texto[i + 3] == 'V'))
            {
              _configuracoes.angulo = (Texto[i + 4] - 48)*100 + (Texto[i + 5] - 48)*10 + (Texto[i + 6] - 48);
              _configuracoes.posicao = (Texto[i + 7] - 48)*10 + (Texto[i + 8] - 48);
              Server.println();
              Server.print("angulo des = ");
              Server.print(_configuracoes.angulo);
              Server.print(" ");
              Server.print("posicao des = ");
              Server.println(_configuracoes.posicao);
            }
          }
        }
      }
    }
  }
}


