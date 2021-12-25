// 정현우 오셀로코드

#include <mega128.h>
#include <delay.h>  
#include <stdio.h>
                
#define FUNCSET         0x28    // Function Set
#define ENTMODE         0x06    // Entry Mode Set
#define ALLCLR          0x01    // All Clear
#define DISPON          0x0c     // Display On
#define LINE2           0xC0    // 2nd Line Move
#define HOME            0x02    // Cursor home
                                
// 효과음과 종료음악을 내기 위한 펄스 주기값
#define DO_            3822     
#define MI_            3214
#define SO_            2551
#define SI_            2145
#define DO             1803  
#define FA             1431
#define SO             1203
                         
//  LCD 관련 함수
void LCD_init(void);                    // LCD 초기설정
void LCD_String1(char flash *str);         // 고정 문자열 LCD 출력 
void LCD_String2(char *str);               // 생성 문자열 LCD  출력      
void Busy(void);                        // 딜레이 함수
void Command(unsigned char);            // LCD 커맨드 출력
void Data(unsigned char);               // LCD 데이터 출력

// 게임 관련 함수
void start_view(void);                  // 초기 도트매트릭스 출력("오델로"문자 출력)
void win(int);                          // 승리 처리 함수 
void game_win(void);                    // 각 돌의 개수 계산 및 게임 승리조건 판단
void draw(void);                        // 비겼을 때의 출력 함수
void game_status(void);                 // 현재 각 돌의 개수를 LCD에 출력하는 함수
void game_rule(int);                    // 인수로 주어진 색이 놓였을때의 오셀로 룰에 따른 알고리즘 검사
void sound_on(void);                    // 돌이 판에 놓였을 때의 효과음 출력
void music_on(void);                    // 게임이 끝난 후 나오는 음악 출력
void timer_setup(void);                 // 타이머/카운터 초기설정

char x, y;                              // 타이머/카운터0으로 도트매트릭스 열,행 좌표값
int a,b;                                // 커서의 이동 및 좌표정보(x,y 변수로 값이 들어감)
int blk = 0;                            // 커서 깜빡이게 하기 위한 변수
int start = 0;                          // 게임 시작 상태 변수?
int end = 0;                            // 게임종료 상태 변수
int tone, time;                         // 음악 출력시 각 음의 정보
int change = 1;                         // 돌 놓는 순서(0 : 녹색, 1 : 적색)
int turn_pass = 0;                      // 선택 스위치 입력수(순서 변경을 위해)
int red, green;                         // 녹색/적색 돌의 수

// 종료 음악 음계 및 음의 길이값
int flash tone_table[15] = {DO_, MI_, SO_, MI_, SO_, SI_, SO_, SI_, DO, SI_, DO, FA, DO, FA, SO};
int flash time_table[15] = {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 48};    
                                
// 좌표에 따른 도트 매트릭스 출력값                                 
char flash pos_bit[8] = {0B00000001, 0B00000010, 0B00000100, 0B00001000,
                         0B00010000, 0B00100000, 0B01000000, 0B10000000}; 
                      
// 오셀로 판 초기값(0 : blank , 1 : green, 2 : red) 
unsigned char dot_view[8][8] = {{0, 0, 0, 0, 0 ,0, 0, 0},
                                {0, 0, 0, 0, 0 ,0, 0, 0},
                                {0, 0, 0, 0, 0 ,0, 0, 0},
                                {0, 0, 0, 0, 0 ,0, 0, 0},
                                {0, 0, 0, 0, 0 ,0, 0, 0},
                                {0, 0, 0, 0, 0 ,0, 0, 0},
                                {0, 0, 0, 0, 0 ,0, 0, 0},
                                {0, 0, 0, 0, 0 ,0, 0, 0}};

void main(void)
{                            
        // 포트 입출력 설정 
        DDRA = 0xFF;            // 포트 A 출력설정(도트매트릭스 green)
        DDRB = 0xFF;            // 포트 B 출력설정(도트매트릭스 com)
    DDRC = 0xFF;            // 포트 C 출력설정(도트매트릭스 red)
        DDRG = 0xFF;            // 포트 D 출력설정(스피커 출력) 
                                                   
    // 외부 인터럽트 초기설정(커서 이동을 위한 스위치 연결)
    EICRA = 0b10101010;     // 외부인터럽트 0,1,2,3 : 하강에지
    EICRB = 0b10101010;     // 외부인터럽트 4,5,6,7 : 하강에지
    EIMSK = 0xFF;            // 외부 인터럽트0~7 인에이블

        timer_setup();       // 타이머/카운터 초기설정 

    LCD_init();                             // LCD 초기설정
    LCD_String1("  WELCOME  TO   ");    // LCD 첫번째 라인에 출력
       Command(LINE2);
       LCD_String1(" OTHELLO  WORLD ");    // LCD 두번째 라인에 출력
                 
    while(1){
                if(start == 0){         // 게임 시작할 때 한번만 실행시키기 위한 변수
                       start_view();    // "오델로" 문자 출력
          }
                game_win();     // 각 돌의 개수를 세고, 게임승리조건 판단              
        }
}
 
// 타이머/카운터 초기설정    
void timer_setup(void)
{
        // 타이머/카운터0 초기설정
        ASSR=0x00;              // 타이머/카운터0 타이머 모드
        TCCR0=0x07;             // 일반모드, 1024분주
        TCNT0=0xE0;             // 주기 = 1/16us X 1024분주 X (256 - 224) = 2048us

        // 타이머/카운터1 초기설정(녹색 선택 스위치 입력 처리)
        TCCR1A = 0;             // 타이머/카운터1 일반모드
        TCCR1B = 0x06;          // T1핀을 통해 입력되는 카운터 모드(하강에지)
        TCCR1C = 0;
        TCNT1 = 0xFFFF;        // 타미어카운터1 초기값 설정(0xFFFF)
                                // T1핀이 한번 눌리면 인터럽트 발생(0xFFFF -> 0x0000)
                                                         
        // 타이머/카운터2 초기설정(적색 선택 스위치 입력 처리)
        TCCR2 = 0b00000110;     // T2핀을 통해 입력되는 카운터 일반모드(하강에지)
        TCNT2 = 0xFF;           // 초기값 0xFF(T2핀이 한번만 눌리면 인터럽트 발생)
                                                                             
        // 타이머/카운터3 초기설정(게임종료 음악 연주를 위한 타이머)
        TCCR3A = 0;             // 타이머/카운터3 일반모드
        TCCR3B = 0x05;          // 1024분주
        TCCR3C = 0;
        TCNT3H = (65440 >> 8) & 0x0FF;     
        TCNT3L = 65540 & 0x0FF;           // 1/16us x 1024 x (65536 - 65440) = 6.14ms
        
        // 타이머/카운터 인터럽트 설정
        TIMSK = 0b01000101;     // 타/카 0,1,2 오버플로우 인터럽트 인에이블
        ETIMSK = 0;             // 타/카3 인터럽트 금지
        SREG = 0x80;            // 전역 인터럽트 인에이블 비트 셋
}

// 초기 화면 출력
void start_view(void)
{
        int     i, j;
                           
        // "오" 문자 출력
        dot_view[6][7] = 1;     
        dot_view[6][6] = 1;
        dot_view[6][5] = 1;
        dot_view[6][4] = 1;
        dot_view[6][3] = 1;
        dot_view[6][2] = 1;
        dot_view[6][1] = 1;
        dot_view[5][4] = 1;
        dot_view[4][5] = 1;
        dot_view[4][4] = 1;
        dot_view[4][3] = 1;
        dot_view[3][2] = 1;
        dot_view[2][2] = 1;
        dot_view[1][3] = 1;
        dot_view[1][4] = 1;
        dot_view[1][5] = 1;
        dot_view[2][6] = 1;
        dot_view[3][6] = 1;
        delay_ms(2000);  
 
        Command(HOME);
    LCD_String1("KUT Electronic.");            // LCD 첫번째 라인에 출력
       Command(LINE2);
       LCD_String1("Prof.SHIN, byLSH");    // LCD 두번째 라인에 출력
       
        //클리어
        for(i=0;i<8;i++){
                for(j=0;j<8;j++) dot_view[i][j] = 0;
        }
        delay_us(1);

        // "델" 문자 출력
        dot_view[0][6] = 2;
        dot_view[1][6] = 2;
        dot_view[2][6] = 2;
        dot_view[0][5] = 2;
        dot_view[1][5] = 2;
        dot_view[2][5] = 2;
        dot_view[3][5] = 2;
        dot_view[4][5] = 2;
        dot_view[5][5] = 2;
        dot_view[7][5] = 2;
        dot_view[1][4] = 2;
        dot_view[3][4] = 2;
        dot_view[5][4] = 2;
        dot_view[7][4] = 2;
        dot_view[0][3] = 2;
        dot_view[2][3] = 2;
        dot_view[3][3] = 2;
        dot_view[5][3] = 2;
        dot_view[7][3] = 2;
        dot_view[0][2] = 2;
        dot_view[2][2] = 2;
        dot_view[3][2] = 2;
        dot_view[5][2] = 2;
        dot_view[6][2] = 2;
        dot_view[7][2] = 2;
        dot_view[0][1] = 2;
        dot_view[1][1] = 2;
        dot_view[2][1] = 2;
        delay_ms(2000);
                 
        
        
        
        for(i = 0;i < 8;i++){
                for(j=0;j<8;j++) dot_view[i][j] = 0;
        }
        delay_us(1);
        
        Command(HOME);
    LCD_String1(" NOW START GAME ");    // LCD 첫번째 라인에 출력
       Command(LINE2);
       LCD_String1(" HAVE A FUN TIME");    // LCD 두번째 라인에 출력
    

        // '로'문자 출력
        dot_view[0][5] = 3;
        dot_view[1][5] = 3;
        dot_view[2][5] = 3;
        dot_view[4][5] = 3;
        dot_view[7][5] = 3;
        dot_view[0][4] = 3;
        dot_view[2][4] = 3;
        dot_view[4][4] = 3;
        dot_view[7][4] = 3;
        dot_view[0][3] = 3;
        dot_view[2][3] = 3;
        dot_view[4][3] = 3;
        dot_view[5][3] = 3;
        dot_view[6][3] = 3;
        dot_view[7][3] = 3;
        dot_view[0][2] = 3;
        dot_view[2][2] = 3;
        dot_view[4][2] = 3;
        dot_view[7][2] = 3;
        dot_view[0][1] = 3;
        dot_view[2][1] = 3;
        dot_view[3][1] = 3;
        dot_view[4][1] = 3;
        dot_view[7][1] = 3;
        delay_ms(2000);
        
        // 게임판 클리어
        for(i=0;i<8;i++){
                for(j=0;j<8;j++) dot_view[i][j] = 0;
        }
        delay_us(1);    
         
        //오셀로 기본 게임판 출력       
        dot_view[3][4] = 2;     
        dot_view[4][3] = 2;
        dot_view[3][3] = 1;
        dot_view[4][4] = 1;
        
        start = 1;    // 게임 시작 상태값 셋?

        a = 2;          // 초기 커서의 x 좌표값
        b = 2;          // 초기 커서의 y 좌표값
} 
                 
// 게임 상태 체크
void game_win(void)
{
        int     i, j;
       
        green = 0;
        red = 0;
                      
        // 녹색/빨강색 알수 카운트
        for(i=0;i<8;i++){
                for(j=0;j<8;j++){
                        if(dot_view[i][j] == 1) green++;
                        else if(dot_view[i][j] == 2) red++;                }
        }

        if(red == 0) win(1);                    // 적색 돌이 하나도 없을 경우 녹색 승리
        else if(green == 0) win(2);             // 녹색 돌이 하나도 없을 경우 적색 승리
        else if((red + green) == 64){           // 판이 꽉 찼을 경우
                if(red > green) win(2);         // 적색이 녹색보다 많으면 적색 승리
                else if(green > red) win(1);    // 녹색이 적색보다 많으면 녹색 승리
                else if(red == green) draw();   // 개수가 같을 경우는 비김.
        }
        else game_status();     // 게임종료상황이 아닐경우 게임상황 출력 함수 호출
}

// 게임상태 표시(녹색/적색 돌의 수 표시)
void game_status(void)
{                                         
        char    dot_buf[16];
         
        // LCD 첫번째 라인에 출력               
        Command(HOME);
        LCD_String1("REDvsGREEN|Turn ");    
        
        // 적색과 녹색 돌의 개수를 두번째줄에 출력
           Command(LINE2);   
        if(change == 0){  // 녹색 순서
                sprintf(dot_buf,"%2d  %2d  |GREEN", red, green); 
                LCD_String2(dot_buf);
       }
       else {           // 적색 순서?
                sprintf(dot_buf,"%2d    %2d  |RED", red, green); 
                LCD_String2(dot_buf);
       } 
}    
 
// 승리 처리함수, 녹색승리 val = 1, 적색 승리 val = 2
void win(int val)
{
        int     i, j;
          
        // LCD 첫번째 라인에 승리 색 출력
        Command(HOME);                                         
        if(val == 1) LCD_String1("   GREEN WIN    ");    // 녹색 승리 
       else LCD_String1("    RED WIN     ");
       
        // LCD 두번째 라인에 출력
       Command(LINE2);
       LCD_String1(" CONGRATULATION ");

        music_on();             // 게임 종료 음악 플레이
        end = 1;                // 게임 종료 플래그 비트 셋
        
        // 게임판 클리어?
        for(i = 0;i < 8;i++){
               for(j = 0;j < 8;j++) dot_view[i][j] = 0;
        }
        delay_us(1);

        // 승리 색으로 비트맨 출력
        for(i=0;i<2;i++){
                dot_view[2][5] = val;
                dot_view[5][5] = val;
                dot_view[6][5] = val;
                dot_view[2][4] = val;
                dot_view[3][4] = val;        
                dot_view[4][4] = val;
                dot_view[2][3] = val;
                dot_view[3][3] = val;
                dot_view[4][3] = val;        
                dot_view[2][2] = val;
                dot_view[5][2] = val;
                dot_view[6][2] = val;        
                dot_view[1][6] = 0;
                dot_view[0][5] = 0;
                dot_view[0][3] = 0;
                dot_view[1][3] = 0;
                dot_view[0][2] = 0;
                dot_view[1][2] = 0;
                dot_view[3][1] = 0;
                dot_view[4][1] = 0;
                dot_view[6][1] = 0;
                dot_view[6][0] = 0;
                dot_view[7][7] = val;
                dot_view[3][6] = val;
                dot_view[4][6] = val;
                dot_view[7][6] = val;
                dot_view[0][5] = val;
                dot_view[1][5] = val;
                dot_view[7][5] = val;
                dot_view[0][4] = val;
                dot_view[1][4] = val;
                dot_view[6][2] = val;
                dot_view[1][1] = val;
                dot_view[7][1] = val;
                dot_view[1][0] = val;
                delay_ms(300);      
                
                dot_view[1][0] = 0;
                dot_view[0][0] = val;
                dot_view[7][5] = 0;
                dot_view[7][6] = 0;
                dot_view[6][6] = val;
                dot_view[6][7] = val;
                dot_view[7][7] = 0;
                delay_ms(300);
                
                dot_view[0][0] = 0;
                dot_view[0][1] = val;
                delay_ms(300);
                
                dot_view[0][1] = 0;
                dot_view[0][2] = val;
                delay_ms(300);
                
                dot_view[6][7] = 0;
                dot_view[3][6] = 0;
                dot_view[4][6] = 0;
                dot_view[6][6] = 0;
                dot_view[0][5] = 0;
                dot_view[1][5] = 0;
                dot_view[0][4] = 0;
                dot_view[1][4] = 0;
                dot_view[0][2] = 0;
                dot_view[7][2] = 0;
                dot_view[1][1] = 0;
                dot_view[7][1] = 0;
                dot_view[0][6] = val;
                dot_view[1][6] = val;
                dot_view[7][6] = val;                
                dot_view[7][5] = val;
                dot_view[0][3] = val;
                dot_view[1][1] = val;
                dot_view[0][2] = val;
                dot_view[1][2] = val;        
                dot_view[3][1] = val;
                dot_view[4][1] = val;
                dot_view[6][1] = val;
                dot_view[6][0] = val;
                delay_ms(300);
                
                dot_view[0][6] = 0;        
                dot_view[0][5] = val;
                delay_ms(300);
                
                dot_view[0][5] = 0;
                dot_view[0][6] = val;
                delay_ms(300);      
                
                dot_view[0][7] = 0;
                dot_view[0][5] = val;
                delay_ms(300);
        }      
        
        // 마름모 출력    
        for(i=0;i<8;i++){
                for(j=0;j<8;j++) dot_view[i][j] = val;
        }

        dot_view[0][7] = 0;
        dot_view[0][6] = 0;
        dot_view[1][7] = 0;
        dot_view[0][5] = 0;
        dot_view[1][6] = 0;
        dot_view[2][7] = 0;
        dot_view[0][0] = 0;
        dot_view[0][1] = 0;
        dot_view[0][2] = 0;
        dot_view[1][0] = 0;
        dot_view[1][1] = 0;
        dot_view[2][0] = 0;
        dot_view[7][0] = 0;
        dot_view[7][1] = 0;
        dot_view[7][2] = 0;
        dot_view[6][0] = 0;
        dot_view[6][1] = 0;
        dot_view[5][0] = 0;
        dot_view[7][7] = 0;
        dot_view[7][6] = 0;
        dot_view[7][5] = 0;
        dot_view[6][7] = 0;
        dot_view[6][6] = 0;
        dot_view[5][7] = 0;
        delay_ms(1000);       
    
        // '승' 출력
        for(i=0;i<8;i++){
                for(j=0;j<8;j++) dot_view[i][j] = 0;
        }

        for(i=0;i<8;i++) dot_view[3][i] = val;        
    
        dot_view[2][6] = val;
        dot_view[6][6] = val;
        dot_view[1][5] = val;
        dot_view[5][5] = val;
        dot_view[7][5] = val;
        dot_view[0][4] = val;
        dot_view[5][4] = val;
        dot_view[7][4] = val;
        dot_view[0][3] = val;
        dot_view[5][3] = val;
        dot_view[7][3] = val;
        dot_view[1][2] = val;
        dot_view[5][2] = val;
        dot_view[7][2] = val;
        dot_view[2][1] = val;
        dot_view[6][1] = val;
    
        delay_ms(2500);

        // 게임판 클리어
        for(i=0;i<8;i++){
                for(j=0;j<8;j++) dot_view[i][j] = 0;
        }
                                                 
        end = 0;     // 게임 종료 플래그 비트 리셋
        start = 0;   // 초기화면 표시부터 다시 시작
}

// 무승부 처리
void draw(void)
{    
        Command(HOME);
    LCD_String1("     draw     ");            // LCD 첫번째 라인에 출력
       Command(LINE2);
       LCD_String1(" congraturation ");    // LCD 두번째 라인에 출력        
        start = 0;   // 초기화면 표시부터 다시 시작
}
        
// 인수로 주어진 col색의 돌일 경우
// 8방향에 대해 오셀로 알고리즘에 따라
// 색이 변할 돌이 있는지 판단
void game_rule(int col)  
{
        int     i, j, aa, bb, aaaa, cross, col1, m, n;

        aa = a;         
        bb = b;
        aaaa = 0;               // 색이 변화하는 돌이 있는지 여부(1 : 색 변하는 돌 있음) ?
        
        if(col == 1) col1 = 2;  // 녹색 돌이 놓인 경우 상대 색 
        else col1 = 1;          // 적색 돌이 놓인 경우 상대 색  

        // 수직 윗방향의 돌들을 판단
        for(i = bb + 1;i < 7;i++) {
                if(dot_view[aa][i] == col1){                    // 돌이 col1 색이면
                        if(dot_view[aa][i + 1] == col){         // 다음 돌이 col 색이면 
                                // col색 돌 전까지의 돌을 모두 col 로 바꾼다.
                                for(j = bb;j < i + 1;j++) dot_view[aa][j] = col;

                                aaaa = 1;    // 다음 차례로 넘기기 위한 변수
                                break;
                        }
                }
                else break;
        }

        // 수평 오른쪽 방향의 돌들을 판단
        for(i = aa + 1;i < 7;i++){
                if(dot_view[i][bb] == col1){                    // 돌이 col1색이면
                        if(dot_view[i + 1][bb] == col){         // 다음 돌이 col색이면
                               // col색 돌전까지의 돌을 모두 col 로 바꾼다.
                                for(j = aa;j < i + 1;j++) dot_view[j][bb] = col;

                                aaaa = 1;    // 다음 차례로 넘기기 위한 변수
                                break;
                        }
                }
                else break;
        }
    
        // 수직 아래쪽 방향의 돌을 판단
        for(i = bb - 1;i > 0;i--){
                 if(dot_view[aa][i] == col1){                   // 돌이 col1 색이면
                        if(dot_view[aa][i - 1] == col){         // 다음 돌이 col 색이면 
                               // col색 돌 전까지의 돌을 모두 col 로 바꾼다.
                                for(j = bb;j > i - 1;j--) dot_view[aa][j] = col;

                                aaaa = 1;    // 다음 차례로 넘기기 위한 변수
                                break;
                        }
                }                     
                else break;
        }

        // 수평 왼쪽 방향의 돌을 판단
        for(i = aa - 1;i > 0;i--){
                  if(dot_view[i][bb] == col1){                  // 돌이 col1색이면
                        if(dot_view[i - 1][bb] == col){         // 다음 돌이 col색이면
                                  // col색 돌 전까지의 돌을 모두 col 로 바꾼다.
                                for(j = aa;j > i - 1;j--) dot_view[j][bb] = col;

                                aaaa = 1;    // 다음 차례로 넘기기 위한 변수
                                break;
                        }
                }
                else break;
        }

        //  오른쪽 위 방향의 돌을 판단(45도 대각선 방향 : y = x + cross)
        cross = bb - aa;
        for(i = aa + 1;i < 7;i++){
                j = i + cross;
                if(j > 6) break;

                if(dot_view[i][j] == col1){                     // 돌이 col1색이면
                        if(dot_view[i+1][j+1] == col){          // 다음 돌이 col색이면
                                  // col색 돌 전까지의 돌을 모두 col 로 바꾼다.
                                for(m = aa;m < i + 1;m++){ 
                                        n = m + cross;
                                        dot_view[m][n] = col;
                                }
                                aaaa = 1;    // 다음 차례로 넘기기 위한 변수
                                break;
                        }
                }
                else break;
        }
            
        //  오른쪽 아래방향의 돌 판단(-45도 대각선 방향 : y = -x + cross)
        cross = bb + aa;
        for(i = aa + 1;i < 7;i++){
                j = cross - i;
                if(j < 1) break;

                if(dot_view[i][j] == col1){                     // 돌이 col1색이면
                        if(dot_view[i + 1][j - 1] == col){      // 다음 돌이 col색이면
                                // col색 돌 전까지의 돌을 모두 col 로 바꾼다.
                                for(m = aa;m < i + 1;m++) {                         
                                        n = cross - m;
                                        dot_view[m][n] = col;
                                }
                                aaaa = 1;    // 다음 차례로 넘기기 위한 변수
                                break;
                        }
                }
                else break;
        }
            
        // 왼쪽 아래방향의 돌 판단(215도 방향 : y = x + cross)
        cross = bb - aa;
        for(i = aa - 1;i > 0;i--){
                j = i + cross;               
                if(j < 1) break;
                
                if(dot_view[i][j] == col1){                     // 돌이 col1색이면
                        if(dot_view[i - 1][j - 1] == col){      // 다음 돌이 col색이면
                                  // col색 돌 전까지의 돌을 모두 col 로 바꾼다.
                                for(m = aa;m > i - 1;m--){
                                        n = m + cross;
                                        dot_view[m][n] = col;
                                }
                                aaaa = 1;    // 다음 차례로 넘기기 위한 변수
                                break;
                        }
                }
                else break;
        }
            
        // 왼쪽 위 방향 돌 판단(135도 대각선 방향 : y = -x + cross)
        cross = bb + aa;    
        for(i = aa - 1;i > 0;i--){
                j = cross - i;
                if(j > 6) break;
                
                if(dot_view[i][j] == col1){                     // 돌이 col1색이면
                        if(dot_view[i-1][j+1] == col){          // 다음 돌이 col색이면
                                // col색 돌 전까지의 돌을 모두 col 로 바꾼다.
                                for(m = aa;m > i - 1;m--){
                                        n = cross - m;
                                        dot_view[m][n] = col;
                                }
                                aaaa = 1;    // 다음 차례로 넘기기 위한 변수
                                break;
                        }
                }
                else break;
        }    
    
        if(aaaa == 1){           
                // 변동 사항이 있으면 순서 적색으로 바꿈?
                if(col == 1) change = 1;
                else change = 0;      

                turn_pass = 0;
                
                sound_on(); // 돌이 놓였을 경우 효과음 출력
        }        
}

// 효과음 출력   
void sound_on(void)     
{
        int     buf, k;

        tone = SO;                  // 솔 
        for(k = 0;k < 10;k++){      // 음 출력
                PORTG = 0xFF;
                buf = tone;
                while(buf--);
        
                PORTG = 0;
                buf = tone;
                while(buf--);
        }

        tone = FA;                  // 파
        for(k=0;k<10;k++){          // 음 출력
                PORTG = 0xFF;
                buf = tone;
                while(buf--);
        
                PORTG = 0;
                buf = tone;
                while(buf--);
        }    
}   

 
// 게임종료 음악 출력(타이머/카운터1 이용)
void music_on(void)     
{
        int     buf, k;
                            
        ETIMSK = 0B00000100;    // 타/카3 오버플로우 인터럽트 인에이블
         
        for(k = 0;k < 15;k++){
                tone = tone_table[k];
                time = time_table[k];
                delay_ms(50);
        
                while(time){
                        PORTG = 0xFF;           // 스피커 출력 on
                        buf = tone;
                        while(buf--);
            
                        PORTG = 0;              // 스피커 출력 off
                        buf = tone;
                        while(buf--);
                }
        }  

        ETIMSK = 0;    // 타/카3 오버플로우 인터럽트 금지?
}    

// 도트매트릭스에 값을 출력
interrupt [TIM0_OVF] void DOT_ON(void) 
{
        PORTA = 0;  // 녹색?
        PORTC = 0;  // 적색
        PORTB = 0;  // Common Cathode

        x++;        // 한 개의 라인씩 출력
        x %= 8;     // 0 - 7

        for(y = 0; y < 8; y++){
                if(dot_view[x][y] == 0){        // 좌표의 배열값이 0이면
                        PORTA |= 0b00000000;    //모두 끈다
                        PORTC |= 0b00000000;            
                }
                else if(dot_view[x][y] == 1){   // 좌표의 배열값이 1이면
                        PORTA |= 0b10000000;    // 녹색만 on(포트A)
                        PORTC |= 0b00000000;
                }
                else if(dot_view[x][y] == 2){   // 좌표의 배열값이 2이면
                        PORTA |= 0b00000000;
                        PORTC |= 0b10000000;    // 빨간색만 on(포트C)
                }            
                else if(dot_view[x][y] == 3){   // 좌표의 배열값이 3이면
                        PORTA |= 0b10000000;    // 모두 on을해서 주황색으로 보이게 한다
                        PORTC |= 0b10000000;
                }
                if(y != 7){
                        PORTA = PORTA >> 1;  // 한행씩 쉬프트   
                        PORTC = PORTC >> 1;
                }
        }        
        
        if(a == x){     // 커서 출력  
              if(blk == 0 && start != 0 && end == 0){
                                PORTC |= pos_bit[b];    // 적색 출력
                                PORTA |= pos_bit[b];    // 녹색 출력
                }
                blk = (blk + 1) % 10;
        } 
        PORTB = pos_bit[x];     //  출력 열 선택

        TCNT0=0xE0;     // 초기값 재설정
}

// 녹색 돌 선택 처리 함수       
interrupt [TIM2_OVF] void select_green(void)
{
    if(change == 1){  
                turn_pass++;    
                if(dot_view[a][b] == 0) game_rule(2);

                if(turn_pass == 5){     // 턴넘김 변수가 5회 눌려 5가 되면
                        change = 0;     // 적색으로 순서 넘김
                    turn_pass = 0;  // 턴넘김변수 초기화
                 }       
        }         
        TCNT2 = 0xFF;          // 초기값 재설정
}

// 적색 돌 선택 처리 함수
interrupt [TIM1_OVF] void select_red(void)
{             
         if(change == 0){ // 녹색의 차례이면
                turn_pass++;
                        
                if(dot_view[a][b] == 0) game_rule(1);

                if(turn_pass == 5){     // 턴넘김 변수가 5회 눌려 5가 되면
                        change = 1;     // 적색으로 순서 넘김
                    turn_pass = 0;  // 턴넘김변수 초기화
                }       
        }
        TCNT1 = 0xFFFF;
}     

// 게임종료 음악 연주를 위한 타이머 함수
interrupt [TIM3_OVF] void gameover_play(void)
{                        
        time--;
        
        TCNT3H = (65440 >> 8) & 0x0FF;     
        TCNT3L = 65540 & 0x0FF;           // 1/16us x 1024 x (65536 - 65440) = 6.14ms
}
                                         
// 녹색 커서 왼쪽 이동 스위치 처리 함수(y 좌표 1감소) 
interrupt [EXT_INT0] void green_move_left(void) 
{
               if(change == 0 && b != 0) b--; 
}
                                         
// 녹색 커서 아래 이동 스위치 처리 함수(x 좌표 1증가)
interrupt [EXT_INT1] void green_move_down(void) 
{  
            if(change == 0 && a != 7) a++;  
} 
                                         
// 녹색 커서 오른쪽 이동 스위치 처리 함수(y 좌표 1증가)  
interrupt [EXT_INT2] void green_move_right(void) 
{          
             
             if(change == 0 && b != 7) b++;
} 
                                         
// 녹색 커서 위 이동 스위치 처리 함수(x 좌표 1감소)  
interrupt [EXT_INT3] void green_move_up(void) 
{                         
           
          if(change == 0 && a != 0) a--;
} 
  
                                         
// 적색 커서 오른쪽 이동 스위치 처리 함수(x 좌표 1증가)
interrupt [EXT_INT4] void red_move_right(void) 
{                                                    
         if(change == 1 && b != 7) b++;
}
                                         
// 적색 커서 위 이동 스위치 처리 함수(y좌표 1감소)
interrupt [EXT_INT5] void red_move_up(void) 
{            
             if(change == 1 && a != 0) a--;
        
}

// 적색 커서 왼쪽 이동 스위치 처리 함수(x 좌표 1감소)  
interrupt [EXT_INT6] void red_move_left(void) 
{            
        
         if(change == 1 && b != 0) b--;
}

// 적색 커서 아래 이동 스위치 처리 함수(y 좌표 1증가)
interrupt [EXT_INT7] void red_move_down(void) 
{            
       
        if(change == 1 && a != 7) a++;
}    

// LCD 초기화  
void LCD_init(void)
{       
    DDRF = 0xFF;            // 포트 F 출력 설정
    PORTF &= 0xFB;            // E = 0;
                  
    delay_ms(15);
    Command(0x20);
    delay_ms(5);
    Command(0x20);
    delay_us(100);
    Command(0x20);
    Command(FUNCSET);
    Command(DISPON);
    Command(ALLCLR);
    Command(ENTMODE);
}

// 문자열 출력 함수
void LCD_String1(char flash *str)
{
    char flash *pStr=0;
    
    pStr = str;    
    while(*pStr) Data(*pStr++);
}

void LCD_String2(char *str)
{
    while(*str) Data(*str++);
}                                     

// 인스트럭션 쓰기 함수
void Command(unsigned char byte)
{
    Busy();

    // 인스트럭션 상위 바이트
    PORTF = (byte & 0xF0);        // 데이터    
    PORTF &= 0xFE;            // RS = 0;           
    PORTF &= 0xFD;            // RW = 0;  
    delay_us(1);     
    PORTF |= 0x04;            // E = 1;
    delay_us(1);
    PORTF &= 0xFB;            // E = 0;

    // 인스트럭션 하위 바이트
        PORTF = ((byte<<4) & 0xF0);    // 데이터
    PORTF &= 0xFE;            // RS = 0;
    PORTF &= 0xFD;            // RW = 0;
    delay_us(1);         
    PORTF |= 0x04;            // E = 1;
    delay_us(1);
    PORTF &= 0xFB;            // E = 0;
}
                    
//데이터 쓰기 함수
void Data(unsigned char byte)
{
    Busy();
        
    // 데이터 상위 바이트
    PORTF = (byte & 0xF0);        // 데이터
    PORTF |= 0x01;            // RS = 1;
    PORTF &= 0xFD;            // RW = 0;
    delay_us(1);         
    PORTF |= 0x04;            // E = 1;
    delay_us(1);
    PORTF &= 0xFB;            // E = 0;

    // 데이터 하위 바이트
    PORTF = ((byte<<4) & 0xF0);      // 데이터
    PORTF |= 0x01;            // RS = 1;
    PORTF &= 0xFD;            // RW = 0;     
    delay_us(1);         
    PORTF |= 0x04;            // E = 1;
    delay_us(1);
    PORTF &= 0xFB;            // E = 0;
}

// Busy Flag Check -> 일반적인 BF를 체크하는 것이 아니라
// 일정한 시간 지연을 이용한다.
void Busy(void)
{
    delay_ms(2);
}


