#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>

int calcParity(char *data) {
    int count = 0;
    int i, j; 
    for (i = 0; i < strlen(data); i++) {
        char c = data[i];
        for (j = 0; j < 8; j++) if ((c >> j) & 1) count++;
    }
    return (count % 2 == 0) ? 0 : 1;
}

int calc2DParity(char *data) {
    unsigned char colParity = 0;
    int i; 
    for (i = 0; i < strlen(data); i++) colParity ^= data[i];
    return (int)colParity;
}

unsigned short calcCRC16(char *data) {
    unsigned short crc = 0xFFFF;
    int len = strlen(data);
    int i; 
    for (i = 0; i < len; i++) {
        unsigned char x = crc >> 8 ^ data[i];
        x ^= x >> 4;
        crc = (crc << 8) ^ ((unsigned short)(x << 12)) ^ ((unsigned short)(x << 5)) ^ ((unsigned short)x);
    }
    return crc;
}

int calcChecksum(char *data) {
    int sum = 0;
    int i;
    for(i = 0; i < strlen(data); i++) sum += data[i];
    return sum % 256;
}

int calcHamming(char *data) {
    if(strlen(data) == 0) return 0;
    unsigned char val = data[0];
    int p1 = (val & 1) ^ ((val >> 1) & 1) ^ ((val >> 3) & 1);
    int p2 = (val & 1) ^ ((val >> 2) & 1) ^ ((val >> 3) & 1);
    int p4 = ((val >> 1) & 1) ^ ((val >> 2) & 1) ^ ((val >> 3) & 1);
    return (p4 << 2) | (p2 << 1) | p1;
}


int getMethodId(char *methodStr) {
    if (strcmp(methodStr, "PARITY") == 0) return 1;
    if (strcmp(methodStr, "2DPARITY") == 0) return 2;
    if (strcmp(methodStr, "CRC16") == 0) return 3;
    if (strcmp(methodStr, "HAMMING") == 0) return 4;
    if (strcmp(methodStr, "CHECKSUM") == 0) return 5;
    return 0;
}

int main() {
    system("title CLIENT 2 (ALICI - SON NOKTA)");
    WSADATA wsa;
    SOCKET s, new_socket;
    struct sockaddr_in server, client;
    int c, recv_size;
    char buffer[2000];

    printf("--- CLIENT 2 BASLATILIYOR ---\n");
    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Winsock baslatilamadi.\n"); system("pause"); return 1;
    }

    s = socket(AF_INET , SOCK_STREAM , 0 );
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 8081 ); 

    if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR) {
        printf("HATA: Port 8081 dolu! Lutfen eski pencereleri kapatin.\n");
        system("pause"); return 1;
    }
    listen(s , 3);

    printf("Sistem hazir. Server bekleniyor...\n");
    c = sizeof(struct sockaddr_in);
    new_socket = accept(s , (struct sockaddr *)&client, &c);
    printf(">> SERVER BAGLANDI! <<\n\n");

    while((recv_size = recv(new_socket , buffer , 2000 , 0)) != SOCKET_ERROR) {
        buffer[recv_size] = '\0';
        
        char rData[1000] = "", method[50] = "", rCheckHex[20] = "";
        
        char *token = strtok(buffer, "|");
        if(token) strcpy(rData, token);
        token = strtok(NULL, "|");
        if(token) strcpy(method, token);
        token = strtok(NULL, "|");
        if(token) strcpy(rCheckHex, token);

        long receivedCheck = strtol(rCheckHex, NULL, 16);
        long computedCheck = 0;
        int methodId = getMethodId(method);

        switch(methodId) {
            case 1: computedCheck = calcParity(rData); break;
            case 2: computedCheck = calc2DParity(rData); break;
            case 3: computedCheck = calcCRC16(rData); break;
            case 4: computedCheck = calcHamming(rData); break;
            case 5: computedCheck = calcChecksum(rData); break;
        }

        printf("-------------------------------------------\n");
        printf("Gelen Veri       : %s\n", rData);
        printf("Yontem           : %s\n", method);
        printf("Gelen Kod (Hex)  : %X\n", receivedCheck);
        printf("Hesaplanan (Hex) : %X\n", computedCheck);

        if (receivedCheck == computedCheck) {
            printf(">>> DURUM: VERI SAGLAM (DATA CORRECT) <<<\n");
        } else {
            printf(">>> DURUM: !!! VERI BOZUK (CORRUPTED) !!! <<<\n");
        }
        printf("-------------------------------------------\n");
    }
    return 0;
}
