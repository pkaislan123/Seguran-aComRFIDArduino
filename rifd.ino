
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>
LiquidCrystal_I2C lcd(0x20, 16, 2);
#include <MemoryLib.h>




#define button 2     //Botão de cadastro no pino 4
#define SDA_RFID 10  //SDA do RFID no pino 10
#define RST_RFID 9   //Reset do RFID no pino 9
#define Buzzer 8
/*----- VARIÁVEIS -----*/
MFRC522 mfrc522(SDA_RFID, RST_RFID);   //Inicializa o módulo RFID
MemoryLib memory(1, 2);                //Inicializa a biblioteca MemoryLib. Parametros: memorySize=1 (1Kb) / type=2 (LONG)
int maxCards = 20; //Cada cartão ocupa duas posições na memória. Para 1Kb será permitido o cadastro de 101 cartões
String cards[20] = {};                //Array com os cartões cadastrados

/*----- SETUP -----*/
void setup() {
  //Configura os pinos

  pinMode(button, INPUT);

  //Inicia SPI
  SPI.begin();

  //Inicia o modulo RFID MFRC522
  mfrc522.PCD_Init();

  Serial.begin(9600);
  lcd.init();              //INICIA O LCD
  lcd.backlight();         //LIGA O BACKLIGHT OU LUZ DE FUNDO

  pinMode(Buzzer, OUTPUT);
  //Retorna os cartões armazenados na memória EEPROM para o array
  ReadMemory();
  printLcd(0, 0, "Bem-Vindo", true, false);
  alertaSonoro(2, 35, 500);
/*
  for (int e = 0; e <= memory.lastAddress; e++) { //Limpa os valores da memória
    memory.write(e, 0);
  }
*/
}

/*----- LOOP PRINCIPAL -----*/
void loop() {

  printLcd(0, 0, "Aguardando...", false, false);

  //Verifica se o botão para castrado de novo cartão foi pressionado
  if (digitalRead(button) == HIGH) {
    RecUser();
  }

  String UID = ReadCard();
  if (UID != "") {
    boolean cadastrado = false;

    printLcd(0, 0, UID, true, false);
    for (int c = 0; c < maxCards; c++) {
      Serial.print("Posicao: ");
      Serial.print(c);
      Serial.print(" Valor no array: ");
      Serial.println(cards[c]);

      if (cards[c] == UID) {
        Serial.println("Cartao Encontrado");
        delay(1000);
        lcd.clear();
        cadastrado = true;
        break;
      }
    }

    if (cadastrado) {
      printLcd(0, 0, "Acesso Aceito", true, false);
      alertaSonoro(2, 100, 500);
      liberarAcesso();
      delay(200);
    } else {
      printLcd(0, 0, "Bloqueado", true, false);
      alertaSonoro(2, 50, 500);
      delay(1000);

    }


  }


}

void liberarAcesso() {
   //acionar o rele para desbloquear a fechadura
}


String ReadCard() {
  String UID = "";
  if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
    //Retorna o UID para a variavel UIDcard
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      UID += String(mfrc522.uid.uidByte[i]);
    }
  }
  return UID;
}

void RecUser() {
  String UID = "";
  boolean deleteCard = false;

  printLcd(0, 0, "Passe a tag", true, false);

  //Faz a leitura o cartao
  do {
    UID = ReadCard();
  } while (UID == "");

  Serial.print("UID lida: ");
  Serial.println(UID);
  if (UID != "") {

    Serial.print("Valor procurado: ");
    Serial.println(UID);
    printLcd(0, 0, UID, true, false);
    for (int c = 0; c < maxCards; c++) {
      Serial.print("Posicao: ");
      Serial.print(c);
      Serial.print(" Valor no array: ");
      Serial.println(cards[c]);

      if (cards[c] == UID) {
        Serial.println("Cartao Ja cadastrado");
        printLcd(0, 1, "Ja Cadastrado", true, true);
        alertaSonoro(2, 50, 500);
        delay(1000);
        lcd.clear();
        return;
      }
    }
  }

  Serial.println("Cartao nao Cadastrado");

  printLcd(0, 0, "Cadastrando...", true, true);

  int posicaoLivre = 0;
  for (int c = 0; c < maxCards; c++) {
    if (cards[c].toInt() == 0) { //Posicao livre
      Serial.println("possicao livre");
      posicaoLivre = c;
      cards[c] = UID;
      break; //finaliza o for
    }
  }


  Serial.print("Valor salvo: ");
  Serial.println(cards[posicaoLivre]);
  Serial.print("Na posicao: ");
  Serial.println(posicaoLivre);


  for (int e = 0; e <= memory.lastAddress; e++) {
    memory.write(e, 0);
  }
  int a = 0;
  for (int c = 0; c < maxCards; c++) {
    if (cards[c].toInt() != 0) {
      memory.write(a, cards[c].substring(0, 6).toInt());
      memory.write(a + 1, cards[c].substring(6, cards[c].length()).toInt());
      a += 2;
    }
  }

  ReadMemory();
  Serial.println("Cadastro Concluido");
  printLcd(0, 0, "Cadastrado", true, true);
  alertaSonoro(2, 100, 500);
  delay(1000);
  lcd.clear();
}


String lerSerial() {

  String uid = "";
  if (Serial.available()) {
    uid = Serial.readString();
  }
  return uid;

}

void printLcd(int coluna, int linha, String mensagem, boolean clearlcd, boolean clearLinha) {

  if (clearlcd)
    lcd.clear();

  if (clearLinha) {
    lcd.setCursor(0, linha);
    lcd.print("                ");
  }

  lcd.setCursor(coluna, linha);
  lcd.print(mensagem);
}

void alertaSonoro(int repeticao, int tempo, int frequencia) {
  for (int i = 0; i < repeticao; i++) {
    tone(Buzzer, frequencia);                                                //LIGO O BUZZER
    delay(tempo);                                                        //MANTÉM LIGADO POR ESSE TEMPO
    noTone(Buzzer);                                                    //DESLIGA O BUZZER
    delay(tempo);
  }
}

void ReadMemory() {
  int a = 0;
  for (int c = 0; c < maxCards; c++) {
    cards[c] = String(memory.read(a)) + String(memory.read(a + 1));
    a += 2;
  }
}
