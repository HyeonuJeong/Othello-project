// ������ �������ڵ�

#include <mega128.h>
#include <delay.h>  
#include <stdio.h>
                
#define FUNCSET         0x28    // Function Set
#define ENTMODE         0x06    // Entry Mode Set
#define ALLCLR          0x01    // All Clear
#define DISPON          0x0c     // Display On
#define LINE2           0xC0    // 2nd Line Move
#define HOME            0x02    // Cursor home
                                
// ȿ������ ���������� ���� ���� �޽� �ֱⰪ
#define DO_            3822     
#define MI_            3214
#define SO_            2551
#define SI_            2145
#define DO             1803  
#define FA             1431
#define SO             1203
                         
//  LCD ���� �Լ�
void LCD_init(void);                    // LCD �ʱ⼳��
void LCD_String1(char flash *str);         // ���� ���ڿ� LCD ��� 
void LCD_String2(char *str);               // ���� ���ڿ� LCD  ���      
void Busy(void);                        // ������ �Լ�
void Command(unsigned char);            // LCD Ŀ�ǵ� ���
void Data(unsigned char);               // LCD ������ ���

// ���� ���� �Լ�
void start_view(void);                  // �ʱ� ��Ʈ��Ʈ���� ���("������"���� ���)
void win(int);                          // �¸� ó�� �Լ� 
void game_win(void);                    // �� ���� ���� ��� �� ���� �¸����� �Ǵ�
void draw(void);                        // ����� ���� ��� �Լ�
void game_status(void);                 // ���� �� ���� ������ LCD�� ����ϴ� �Լ�
void game_rule(int);                    // �μ��� �־��� ���� ���������� ������ �꿡 ���� �˰��� �˻�
void sound_on(void);                    // ���� �ǿ� ������ ���� ȿ���� ���
void music_on(void);                    // ������ ���� �� ������ ���� ���
void timer_setup(void);                 // Ÿ�̸�/ī���� �ʱ⼳��

char x, y;                              // Ÿ�̸�/ī����0���� ��Ʈ��Ʈ���� ��,�� ��ǥ��
int a,b;                                // Ŀ���� �̵� �� ��ǥ����(x,y ������ ���� ��)
int blk = 0;                            // Ŀ�� �����̰� �ϱ� ���� ����
int start = 0;                          // ���� ���� ���� ����?
int end = 0;                            // �������� ���� ����
int tone, time;                         // ���� ��½� �� ���� ����
int change = 1;                         // �� ���� ����(0 : ���, 1 : ����)
int turn_pass = 0;                      // ���� ����ġ �Է¼�(���� ������ ����)
int red, green;                         // ���/���� ���� ��

// ���� ���� ���� �� ���� ���̰�
int flash tone_table[15] = {DO_, MI_, SO_, MI_, SO_, SI_, SO_, SI_, DO, SI_, DO, FA, DO, FA, SO};
int flash time_table[15] = {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 48};    
                                
// ��ǥ�� ���� ��Ʈ ��Ʈ���� ��°�                                 
char flash pos_bit[8] = {0B00000001, 0B00000010, 0B00000100, 0B00001000,
                         0B00010000, 0B00100000, 0B01000000, 0B10000000}; 
                      
// ������ �� �ʱⰪ(0 : blank , 1 : green, 2 : red) 
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
        // ��Ʈ ����� ���� 
        DDRA = 0xFF;            // ��Ʈ A ��¼���(��Ʈ��Ʈ���� green)
        DDRB = 0xFF;            // ��Ʈ B ��¼���(��Ʈ��Ʈ���� com)
    DDRC = 0xFF;            // ��Ʈ C ��¼���(��Ʈ��Ʈ���� red)
        DDRG = 0xFF;            // ��Ʈ D ��¼���(����Ŀ ���) 
                                                   
    // �ܺ� ���ͷ�Ʈ �ʱ⼳��(Ŀ�� �̵��� ���� ����ġ ����)
    EICRA = 0b10101010;     // �ܺ����ͷ�Ʈ 0,1,2,3 : �ϰ�����
    EICRB = 0b10101010;     // �ܺ����ͷ�Ʈ 4,5,6,7 : �ϰ�����
    EIMSK = 0xFF;            // �ܺ� ���ͷ�Ʈ0~7 �ο��̺�

        timer_setup();       // Ÿ�̸�/ī���� �ʱ⼳�� 

    LCD_init();                             // LCD �ʱ⼳��
    LCD_String1("  WELCOME  TO   ");    // LCD ù��° ���ο� ���
       Command(LINE2);
       LCD_String1(" OTHELLO  WORLD ");    // LCD �ι�° ���ο� ���
                 
    while(1){
                if(start == 0){         // ���� ������ �� �ѹ��� �����Ű�� ���� ����
                       start_view();    // "������" ���� ���
          }
                game_win();     // �� ���� ������ ����, ���ӽ¸����� �Ǵ�              
        }
}
 
// Ÿ�̸�/ī���� �ʱ⼳��    
void timer_setup(void)
{
        // Ÿ�̸�/ī����0 �ʱ⼳��
        ASSR=0x00;              // Ÿ�̸�/ī����0 Ÿ�̸� ���
        TCCR0=0x07;             // �Ϲݸ��, 1024����
        TCNT0=0xE0;             // �ֱ� = 1/16us X 1024���� X (256 - 224) = 2048us

        // Ÿ�̸�/ī����1 �ʱ⼳��(��� ���� ����ġ �Է� ó��)
        TCCR1A = 0;             // Ÿ�̸�/ī����1 �Ϲݸ��
        TCCR1B = 0x06;          // T1���� ���� �ԷµǴ� ī���� ���(�ϰ�����)
        TCCR1C = 0;
        TCNT1 = 0xFFFF;        // Ÿ�̾�ī����1 �ʱⰪ ����(0xFFFF)
                                // T1���� �ѹ� ������ ���ͷ�Ʈ �߻�(0xFFFF -> 0x0000)
                                                         
        // Ÿ�̸�/ī����2 �ʱ⼳��(���� ���� ����ġ �Է� ó��)
        TCCR2 = 0b00000110;     // T2���� ���� �ԷµǴ� ī���� �Ϲݸ��(�ϰ�����)
        TCNT2 = 0xFF;           // �ʱⰪ 0xFF(T2���� �ѹ��� ������ ���ͷ�Ʈ �߻�)
                                                                             
        // Ÿ�̸�/ī����3 �ʱ⼳��(�������� ���� ���ָ� ���� Ÿ�̸�)
        TCCR3A = 0;             // Ÿ�̸�/ī����3 �Ϲݸ��
        TCCR3B = 0x05;          // 1024����
        TCCR3C = 0;
        TCNT3H = (65440 >> 8) & 0x0FF;     
        TCNT3L = 65540 & 0x0FF;           // 1/16us x 1024 x (65536 - 65440) = 6.14ms
        
        // Ÿ�̸�/ī���� ���ͷ�Ʈ ����
        TIMSK = 0b01000101;     // Ÿ/ī 0,1,2 �����÷ο� ���ͷ�Ʈ �ο��̺�
        ETIMSK = 0;             // Ÿ/ī3 ���ͷ�Ʈ ����
        SREG = 0x80;            // ���� ���ͷ�Ʈ �ο��̺� ��Ʈ ��
}

// �ʱ� ȭ�� ���
void start_view(void)
{
        int     i, j;
                           
        // "��" ���� ���
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
    LCD_String1("KUT Electronic.");            // LCD ù��° ���ο� ���
       Command(LINE2);
       LCD_String1("Prof.SHIN, byLSH");    // LCD �ι�° ���ο� ���
       
        //Ŭ����
        for(i=0;i<8;i++){
                for(j=0;j<8;j++) dot_view[i][j] = 0;
        }
        delay_us(1);

        // "��" ���� ���
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
    LCD_String1(" NOW START GAME ");    // LCD ù��° ���ο� ���
       Command(LINE2);
       LCD_String1(" HAVE A FUN TIME");    // LCD �ι�° ���ο� ���
    

        // '��'���� ���
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
        
        // ������ Ŭ����
        for(i=0;i<8;i++){
                for(j=0;j<8;j++) dot_view[i][j] = 0;
        }
        delay_us(1);    
         
        //������ �⺻ ������ ���       
        dot_view[3][4] = 2;     
        dot_view[4][3] = 2;
        dot_view[3][3] = 1;
        dot_view[4][4] = 1;
        
        start = 1;    // ���� ���� ���°� ��?

        a = 2;          // �ʱ� Ŀ���� x ��ǥ��
        b = 2;          // �ʱ� Ŀ���� y ��ǥ��
} 
                 
// ���� ���� üũ
void game_win(void)
{
        int     i, j;
       
        green = 0;
        red = 0;
                      
        // ���/������ �˼� ī��Ʈ
        for(i=0;i<8;i++){
                for(j=0;j<8;j++){
                        if(dot_view[i][j] == 1) green++;
                        else if(dot_view[i][j] == 2) red++;                }
        }

        if(red == 0) win(1);                    // ���� ���� �ϳ��� ���� ��� ��� �¸�
        else if(green == 0) win(2);             // ��� ���� �ϳ��� ���� ��� ���� �¸�
        else if((red + green) == 64){           // ���� �� á�� ���
                if(red > green) win(2);         // ������ ������� ������ ���� �¸�
                else if(green > red) win(1);    // ����� �������� ������ ��� �¸�
                else if(red == green) draw();   // ������ ���� ���� ���.
        }
        else game_status();     // ���������Ȳ�� �ƴҰ�� ���ӻ�Ȳ ��� �Լ� ȣ��
}

// ���ӻ��� ǥ��(���/���� ���� �� ǥ��)
void game_status(void)
{                                         
        char    dot_buf[16];
         
        // LCD ù��° ���ο� ���               
        Command(HOME);
        LCD_String1("REDvsGREEN|Turn ");    
        
        // ������ ��� ���� ������ �ι�°�ٿ� ���
           Command(LINE2);   
        if(change == 0){  // ��� ����
                sprintf(dot_buf,"%2d  %2d  |GREEN", red, green); 
                LCD_String2(dot_buf);
       }
       else {           // ���� ����?
                sprintf(dot_buf,"%2d    %2d  |RED", red, green); 
                LCD_String2(dot_buf);
       } 
}    
 
// �¸� ó���Լ�, ����¸� val = 1, ���� �¸� val = 2
void win(int val)
{
        int     i, j;
          
        // LCD ù��° ���ο� �¸� �� ���
        Command(HOME);                                         
        if(val == 1) LCD_String1("   GREEN WIN    ");    // ��� �¸� 
       else LCD_String1("    RED WIN     ");
       
        // LCD �ι�° ���ο� ���
       Command(LINE2);
       LCD_String1(" CONGRATULATION ");

        music_on();             // ���� ���� ���� �÷���
        end = 1;                // ���� ���� �÷��� ��Ʈ ��
        
        // ������ Ŭ����?
        for(i = 0;i < 8;i++){
               for(j = 0;j < 8;j++) dot_view[i][j] = 0;
        }
        delay_us(1);

        // �¸� ������ ��Ʈ�� ���
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
        
        // ������ ���    
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
    
        // '��' ���
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

        // ������ Ŭ����
        for(i=0;i<8;i++){
                for(j=0;j<8;j++) dot_view[i][j] = 0;
        }
                                                 
        end = 0;     // ���� ���� �÷��� ��Ʈ ����
        start = 0;   // �ʱ�ȭ�� ǥ�ú��� �ٽ� ����
}

// ���º� ó��
void draw(void)
{    
        Command(HOME);
    LCD_String1("     draw     ");            // LCD ù��° ���ο� ���
       Command(LINE2);
       LCD_String1(" congraturation ");    // LCD �ι�° ���ο� ���        
        start = 0;   // �ʱ�ȭ�� ǥ�ú��� �ٽ� ����
}
        
// �μ��� �־��� col���� ���� ���
// 8���⿡ ���� ������ �˰��� ����
// ���� ���� ���� �ִ��� �Ǵ�
void game_rule(int col)  
{
        int     i, j, aa, bb, aaaa, cross, col1, m, n;

        aa = a;         
        bb = b;
        aaaa = 0;               // ���� ��ȭ�ϴ� ���� �ִ��� ����(1 : �� ���ϴ� �� ����) ?
        
        if(col == 1) col1 = 2;  // ��� ���� ���� ��� ��� �� 
        else col1 = 1;          // ���� ���� ���� ��� ��� ��  

        // ���� �������� ������ �Ǵ�
        for(i = bb + 1;i < 7;i++) {
                if(dot_view[aa][i] == col1){                    // ���� col1 ���̸�
                        if(dot_view[aa][i + 1] == col){         // ���� ���� col ���̸� 
                                // col�� �� �������� ���� ��� col �� �ٲ۴�.
                                for(j = bb;j < i + 1;j++) dot_view[aa][j] = col;

                                aaaa = 1;    // ���� ���ʷ� �ѱ�� ���� ����
                                break;
                        }
                }
                else break;
        }

        // ���� ������ ������ ������ �Ǵ�
        for(i = aa + 1;i < 7;i++){
                if(dot_view[i][bb] == col1){                    // ���� col1���̸�
                        if(dot_view[i + 1][bb] == col){         // ���� ���� col���̸�
                               // col�� ���������� ���� ��� col �� �ٲ۴�.
                                for(j = aa;j < i + 1;j++) dot_view[j][bb] = col;

                                aaaa = 1;    // ���� ���ʷ� �ѱ�� ���� ����
                                break;
                        }
                }
                else break;
        }
    
        // ���� �Ʒ��� ������ ���� �Ǵ�
        for(i = bb - 1;i > 0;i--){
                 if(dot_view[aa][i] == col1){                   // ���� col1 ���̸�
                        if(dot_view[aa][i - 1] == col){         // ���� ���� col ���̸� 
                               // col�� �� �������� ���� ��� col �� �ٲ۴�.
                                for(j = bb;j > i - 1;j--) dot_view[aa][j] = col;

                                aaaa = 1;    // ���� ���ʷ� �ѱ�� ���� ����
                                break;
                        }
                }                     
                else break;
        }

        // ���� ���� ������ ���� �Ǵ�
        for(i = aa - 1;i > 0;i--){
                  if(dot_view[i][bb] == col1){                  // ���� col1���̸�
                        if(dot_view[i - 1][bb] == col){         // ���� ���� col���̸�
                                  // col�� �� �������� ���� ��� col �� �ٲ۴�.
                                for(j = aa;j > i - 1;j--) dot_view[j][bb] = col;

                                aaaa = 1;    // ���� ���ʷ� �ѱ�� ���� ����
                                break;
                        }
                }
                else break;
        }

        //  ������ �� ������ ���� �Ǵ�(45�� �밢�� ���� : y = x + cross)
        cross = bb - aa;
        for(i = aa + 1;i < 7;i++){
                j = i + cross;
                if(j > 6) break;

                if(dot_view[i][j] == col1){                     // ���� col1���̸�
                        if(dot_view[i+1][j+1] == col){          // ���� ���� col���̸�
                                  // col�� �� �������� ���� ��� col �� �ٲ۴�.
                                for(m = aa;m < i + 1;m++){ 
                                        n = m + cross;
                                        dot_view[m][n] = col;
                                }
                                aaaa = 1;    // ���� ���ʷ� �ѱ�� ���� ����
                                break;
                        }
                }
                else break;
        }
            
        //  ������ �Ʒ������� �� �Ǵ�(-45�� �밢�� ���� : y = -x + cross)
        cross = bb + aa;
        for(i = aa + 1;i < 7;i++){
                j = cross - i;
                if(j < 1) break;

                if(dot_view[i][j] == col1){                     // ���� col1���̸�
                        if(dot_view[i + 1][j - 1] == col){      // ���� ���� col���̸�
                                // col�� �� �������� ���� ��� col �� �ٲ۴�.
                                for(m = aa;m < i + 1;m++) {                         
                                        n = cross - m;
                                        dot_view[m][n] = col;
                                }
                                aaaa = 1;    // ���� ���ʷ� �ѱ�� ���� ����
                                break;
                        }
                }
                else break;
        }
            
        // ���� �Ʒ������� �� �Ǵ�(215�� ���� : y = x + cross)
        cross = bb - aa;
        for(i = aa - 1;i > 0;i--){
                j = i + cross;               
                if(j < 1) break;
                
                if(dot_view[i][j] == col1){                     // ���� col1���̸�
                        if(dot_view[i - 1][j - 1] == col){      // ���� ���� col���̸�
                                  // col�� �� �������� ���� ��� col �� �ٲ۴�.
                                for(m = aa;m > i - 1;m--){
                                        n = m + cross;
                                        dot_view[m][n] = col;
                                }
                                aaaa = 1;    // ���� ���ʷ� �ѱ�� ���� ����
                                break;
                        }
                }
                else break;
        }
            
        // ���� �� ���� �� �Ǵ�(135�� �밢�� ���� : y = -x + cross)
        cross = bb + aa;    
        for(i = aa - 1;i > 0;i--){
                j = cross - i;
                if(j > 6) break;
                
                if(dot_view[i][j] == col1){                     // ���� col1���̸�
                        if(dot_view[i-1][j+1] == col){          // ���� ���� col���̸�
                                // col�� �� �������� ���� ��� col �� �ٲ۴�.
                                for(m = aa;m > i - 1;m--){
                                        n = cross - m;
                                        dot_view[m][n] = col;
                                }
                                aaaa = 1;    // ���� ���ʷ� �ѱ�� ���� ����
                                break;
                        }
                }
                else break;
        }    
    
        if(aaaa == 1){           
                // ���� ������ ������ ���� �������� �ٲ�?
                if(col == 1) change = 1;
                else change = 0;      

                turn_pass = 0;
                
                sound_on(); // ���� ������ ��� ȿ���� ���
        }        
}

// ȿ���� ���   
void sound_on(void)     
{
        int     buf, k;

        tone = SO;                  // �� 
        for(k = 0;k < 10;k++){      // �� ���
                PORTG = 0xFF;
                buf = tone;
                while(buf--);
        
                PORTG = 0;
                buf = tone;
                while(buf--);
        }

        tone = FA;                  // ��
        for(k=0;k<10;k++){          // �� ���
                PORTG = 0xFF;
                buf = tone;
                while(buf--);
        
                PORTG = 0;
                buf = tone;
                while(buf--);
        }    
}   

 
// �������� ���� ���(Ÿ�̸�/ī����1 �̿�)
void music_on(void)     
{
        int     buf, k;
                            
        ETIMSK = 0B00000100;    // Ÿ/ī3 �����÷ο� ���ͷ�Ʈ �ο��̺�
         
        for(k = 0;k < 15;k++){
                tone = tone_table[k];
                time = time_table[k];
                delay_ms(50);
        
                while(time){
                        PORTG = 0xFF;           // ����Ŀ ��� on
                        buf = tone;
                        while(buf--);
            
                        PORTG = 0;              // ����Ŀ ��� off
                        buf = tone;
                        while(buf--);
                }
        }  

        ETIMSK = 0;    // Ÿ/ī3 �����÷ο� ���ͷ�Ʈ ����?
}    

// ��Ʈ��Ʈ������ ���� ���
interrupt [TIM0_OVF] void DOT_ON(void) 
{
        PORTA = 0;  // ���?
        PORTC = 0;  // ����
        PORTB = 0;  // Common Cathode

        x++;        // �� ���� ���ξ� ���
        x %= 8;     // 0 - 7

        for(y = 0; y < 8; y++){
                if(dot_view[x][y] == 0){        // ��ǥ�� �迭���� 0�̸�
                        PORTA |= 0b00000000;    //��� ����
                        PORTC |= 0b00000000;            
                }
                else if(dot_view[x][y] == 1){   // ��ǥ�� �迭���� 1�̸�
                        PORTA |= 0b10000000;    // ����� on(��ƮA)
                        PORTC |= 0b00000000;
                }
                else if(dot_view[x][y] == 2){   // ��ǥ�� �迭���� 2�̸�
                        PORTA |= 0b00000000;
                        PORTC |= 0b10000000;    // �������� on(��ƮC)
                }            
                else if(dot_view[x][y] == 3){   // ��ǥ�� �迭���� 3�̸�
                        PORTA |= 0b10000000;    // ��� on���ؼ� ��Ȳ������ ���̰� �Ѵ�
                        PORTC |= 0b10000000;
                }
                if(y != 7){
                        PORTA = PORTA >> 1;  // ���྿ ����Ʈ   
                        PORTC = PORTC >> 1;
                }
        }        
        
        if(a == x){     // Ŀ�� ���  
              if(blk == 0 && start != 0 && end == 0){
                                PORTC |= pos_bit[b];    // ���� ���
                                PORTA |= pos_bit[b];    // ��� ���
                }
                blk = (blk + 1) % 10;
        } 
        PORTB = pos_bit[x];     //  ��� �� ����

        TCNT0=0xE0;     // �ʱⰪ �缳��
}

// ��� �� ���� ó�� �Լ�       
interrupt [TIM2_OVF] void select_green(void)
{
    if(change == 1){  
                turn_pass++;    
                if(dot_view[a][b] == 0) game_rule(2);

                if(turn_pass == 5){     // �ϳѱ� ������ 5ȸ ���� 5�� �Ǹ�
                        change = 0;     // �������� ���� �ѱ�
                    turn_pass = 0;  // �ϳѱ躯�� �ʱ�ȭ
                 }       
        }         
        TCNT2 = 0xFF;          // �ʱⰪ �缳��
}

// ���� �� ���� ó�� �Լ�
interrupt [TIM1_OVF] void select_red(void)
{             
         if(change == 0){ // ����� �����̸�
                turn_pass++;
                        
                if(dot_view[a][b] == 0) game_rule(1);

                if(turn_pass == 5){     // �ϳѱ� ������ 5ȸ ���� 5�� �Ǹ�
                        change = 1;     // �������� ���� �ѱ�
                    turn_pass = 0;  // �ϳѱ躯�� �ʱ�ȭ
                }       
        }
        TCNT1 = 0xFFFF;
}     

// �������� ���� ���ָ� ���� Ÿ�̸� �Լ�
interrupt [TIM3_OVF] void gameover_play(void)
{                        
        time--;
        
        TCNT3H = (65440 >> 8) & 0x0FF;     
        TCNT3L = 65540 & 0x0FF;           // 1/16us x 1024 x (65536 - 65440) = 6.14ms
}
                                         
// ��� Ŀ�� ���� �̵� ����ġ ó�� �Լ�(y ��ǥ 1����) 
interrupt [EXT_INT0] void green_move_left(void) 
{
               if(change == 0 && b != 0) b--; 
}
                                         
// ��� Ŀ�� �Ʒ� �̵� ����ġ ó�� �Լ�(x ��ǥ 1����)
interrupt [EXT_INT1] void green_move_down(void) 
{  
            if(change == 0 && a != 7) a++;  
} 
                                         
// ��� Ŀ�� ������ �̵� ����ġ ó�� �Լ�(y ��ǥ 1����)  
interrupt [EXT_INT2] void green_move_right(void) 
{          
             
             if(change == 0 && b != 7) b++;
} 
                                         
// ��� Ŀ�� �� �̵� ����ġ ó�� �Լ�(x ��ǥ 1����)  
interrupt [EXT_INT3] void green_move_up(void) 
{                         
           
          if(change == 0 && a != 0) a--;
} 
  
                                         
// ���� Ŀ�� ������ �̵� ����ġ ó�� �Լ�(x ��ǥ 1����)
interrupt [EXT_INT4] void red_move_right(void) 
{                                                    
         if(change == 1 && b != 7) b++;
}
                                         
// ���� Ŀ�� �� �̵� ����ġ ó�� �Լ�(y��ǥ 1����)
interrupt [EXT_INT5] void red_move_up(void) 
{            
             if(change == 1 && a != 0) a--;
        
}

// ���� Ŀ�� ���� �̵� ����ġ ó�� �Լ�(x ��ǥ 1����)  
interrupt [EXT_INT6] void red_move_left(void) 
{            
        
         if(change == 1 && b != 0) b--;
}

// ���� Ŀ�� �Ʒ� �̵� ����ġ ó�� �Լ�(y ��ǥ 1����)
interrupt [EXT_INT7] void red_move_down(void) 
{            
       
        if(change == 1 && a != 7) a++;
}    

// LCD �ʱ�ȭ  
void LCD_init(void)
{       
    DDRF = 0xFF;            // ��Ʈ F ��� ����
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

// ���ڿ� ��� �Լ�
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

// �ν�Ʈ���� ���� �Լ�
void Command(unsigned char byte)
{
    Busy();

    // �ν�Ʈ���� ���� ����Ʈ
    PORTF = (byte & 0xF0);        // ������    
    PORTF &= 0xFE;            // RS = 0;           
    PORTF &= 0xFD;            // RW = 0;  
    delay_us(1);     
    PORTF |= 0x04;            // E = 1;
    delay_us(1);
    PORTF &= 0xFB;            // E = 0;

    // �ν�Ʈ���� ���� ����Ʈ
        PORTF = ((byte<<4) & 0xF0);    // ������
    PORTF &= 0xFE;            // RS = 0;
    PORTF &= 0xFD;            // RW = 0;
    delay_us(1);         
    PORTF |= 0x04;            // E = 1;
    delay_us(1);
    PORTF &= 0xFB;            // E = 0;
}
                    
//������ ���� �Լ�
void Data(unsigned char byte)
{
    Busy();
        
    // ������ ���� ����Ʈ
    PORTF = (byte & 0xF0);        // ������
    PORTF |= 0x01;            // RS = 1;
    PORTF &= 0xFD;            // RW = 0;
    delay_us(1);         
    PORTF |= 0x04;            // E = 1;
    delay_us(1);
    PORTF &= 0xFB;            // E = 0;

    // ������ ���� ����Ʈ
    PORTF = ((byte<<4) & 0xF0);      // ������
    PORTF |= 0x01;            // RS = 1;
    PORTF &= 0xFD;            // RW = 0;     
    delay_us(1);         
    PORTF |= 0x04;            // E = 1;
    delay_us(1);
    PORTF &= 0xFB;            // E = 0;
}

// Busy Flag Check -> �Ϲ����� BF�� üũ�ϴ� ���� �ƴ϶�
// ������ �ð� ������ �̿��Ѵ�.
void Busy(void)
{
    delay_ms(2);
}


