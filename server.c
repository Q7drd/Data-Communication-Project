#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <winsock2.h>

void injectError(char *packet, int errorType) {
    char *firstPipe = strchr(packet, '|');
    if (firstPipe == NULL) return; 

    int dataLen = firstPipe - packet;
    char dataPart[1000];
    char restOfPacket[1000];

    strncpy(dataPart, packet, dataLen);
    dataPart[dataLen] = '\0'; 
    strcpy(restOfPacket, firstPipe);

    if (dataLen == 0) return;

    int pos, bitPos;
    char temp;
    int i;
    switch(errorType) {
        case 1: 
            pos = rand() % dataLen;
            bitPos = rand() % 8;
            dataPart[pos] ^= (1 << bitPos);
            printf(">> [Bit Flip] Index %d, Bit %d degisti.\n", pos, bitPos);
            break;
        case 2: 
            pos = rand() % dataLen;
            dataPart[pos] = 'A' + (rand() % 26);
            printf(">> [Substitution] Index %d degisti.\n", pos);
            break;
        case 3:
            if (dataLen > 0) {
                pos = rand() % dataLen;
                memmove(&dataPart[pos], &dataPart[pos+1], dataLen - pos);
                printf(">> [Deletion] Index %d silindi.\n", pos);
            }
            break;
        case 4: 
            pos = rand() % (dataLen + 1);
            memmove(&dataPart[pos+1], &dataPart[pos], dataLen - pos + 1);
            dataPart[pos] = 'X'; 
            printf(">> [Insertion] Index %d'ye 'X' eklendi.\n", pos);
            break;
        case 5: 
            if (dataLen > 1) {
                pos = rand() % (dataLen - 1);
                temp = dataPart[pos]; dataPart[pos] = dataPart[pos+1]; dataPart[pos+1] = temp;
                printf(">> [Swapping] Index %d ve %d yer degisti.\n", pos, pos+1);
            }
            break;
        case 6: 
        
            for( i=0; i<3; i++) {
                pos = rand() % dataLen;
                dataPart[pos] ^= (1 << (rand() % 8));
            }
            printf(">> [Multi Flip] 3 bit bozuldu.\n");
            break;
        case 7: 
            if (dataLen > 0) {
                int burst = 3;
                pos = rand() % dataLen;
                for( i=0; i<burst && (pos+i)<dataLen; i++) dataPart[pos+i] = '#';
                printf(">> [Burst] Index %d'den itibaren bozuldu.\n", pos);
            }
            break;
    }
    
    sprintf(packet, "%s%s", dataPart, restOfPacket);
}

int main() {
    system("title SERVER (HATA BOZUCU)");
    WSADATA wsa;
    SOCKET s_server, s_client1, s_client2;
    struct sockaddr_in server_addr, c2_addr, c1_addr;
    int c, recv_size, secim;
    char buffer[2000];

    srand(time(NULL)); 
    printf("--- SERVER BASLATILIYOR ---\n");
    WSAStartup(MAKEWORD(2,2), &wsa);

    s_client2 = socket(AF_INET , SOCK_STREAM , 0 );
    c2_addr.sin_family = AF_INET;
    c2_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    c2_addr.sin_port = htons( 8081 );

    printf("Client 2 araniyor...\n");
    while (connect(s_client2 , (struct sockaddr *)&c2_addr , sizeof(c2_addr)) < 0) {
        printf("HATA: Client 2 acik degil! Bekleniyor...\n");
        Sleep(2000);
    }
    printf("Client 2 BAGLANDI.\n");

    s_server = socket(AF_INET , SOCK_STREAM , 0 );
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons( 8888 );

    bind(s_server ,(struct sockaddr *)&server_addr , sizeof(server_addr));
    listen(s_server , 3);

    printf("Client 1 bekleniyor (Port 8888)...\n");
    c = sizeof(struct sockaddr_in);
    s_client1 = accept(s_server , (struct sockaddr *)&c1_addr, &c);
    printf(">> CLIENT 1 BAGLANDI! <<\n");

    while((recv_size = recv(s_client1 , buffer , 2000 , 0)) != SOCKET_ERROR) {
        buffer[recv_size] = '\0';
        printf("\n----------------------------------------\n");
        printf("GELEN: %s\n", buffer);
        printf("[0] Ilet (NO ERROR)\n[1] Bit Flip\n[2] Substitution\n[3] Deletion\n[4] Insertion\n[5] Swapping\n[6] Multi Flip\n[7] Burst Error\nSecim: ");
        scanf("%d", &secim);

        if (secim > 0 && secim <= 7) injectError(buffer, secim);
        
        send(s_client2 , buffer , strlen(buffer) , 0);
        printf("Paket iletildi.\n");
    }
    return 0;
}
