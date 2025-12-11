#include "nextion.h"

void update_nextion(int *page, int *distance1, int *distance2, int *time1, int *time2, float *progressbar, float *total_distance){
    char valtype;

    if(*page == 0){
        
        uint32_t val = read_nextion_value(&valtype);
        //TOUCH EVENT (0x65)
        if(valtype == 0x65){
            switch (val)
            {
                // ID 1 == Start Button / b1
            case 0x01:
                *page = 1;
                
                printf("page 1%c%c%c", 255,255,255);

                printf("page1.pb1.val=%d%c%c%c", *time1+*time2, 255,255,255);
                
                break;

                //ID 5 == Slider 1 / s1
            case 0x05:
                printf("get %s.val%c%c%c", "page0.s1", 255,255,255);
                val = read_nextion_value(&valtype);
                if(valtype == 0x71) {
                    printf("page0.s1.val=%d%c%c%c", (int)val, 255,255,255);
                    printf("page0.d1.val=%d%c%c%c", (int)val, 255,255,255);
                    *distance1 = val;
                }
                break;

                //ID 7 == Slider 2 / s2
            case 0x07:
                printf("get %s.val%c%c%c", "page0.s2", 255,255,255);
                val = read_nextion_value(&valtype);
                if(valtype == 0x71) {
                    printf("page0.s2.val=%d%c%c%c", (int)val, 255,255,255);
                    printf("page0.d2.val=%d%c%c%c", (int)val, 255,255,255);
                    *distance2 = val;
                }
                break;

            default:
                break;
            }
        }
    }
    
    if(*page == 1){
        printf("get %s.val%c%c%c", "page1.pb1", 255,255,255);
        uint32_t val = read_nextion_value(&valtype);

        if(valtype == 0x71) {
            if(*progressbar >= 100){
                //*page = 0;
                //printf("page 0%c%c%c", 255,255,255);
            }else{
                *progressbar += ((int)(*total_distance / (*distance1 + *distance2))) * 100;   // calculates the progress % based on the distance
                printf("page1.pb1.val=%d%c%c%c", (int) *progressbar, 255,255,255);
            }

        }
    }
}

uint32_t read_nextion_value(char* valtype)
{
    uint8_t buf[32];
    while (1)
    {
        int len = read_nextion_message(buf, sizeof(buf));
       
        // If no message arrived, just wait for the next one
        if (len == 0)
            continue;
        
        *valtype = buf[0];
        
        uint32_t value;
        

        switch (buf[0])
        {  
            // NUMERIC RETURN (0x71)
        case 0x71:
            if (len < 8) continue;
            // 71 vv vv vv vv FF FF FF
            value = (uint32_t)buf[1] | ((uint32_t)buf[2] << 8) | ((uint32_t)buf[3] << 16) | ((uint32_t)buf[4] << 24);
            return value;
            break;

            // STRING RETURN (0x70)
        case 0x70:
            value = (uint32_t)buf[1] | ((uint32_t)buf[2] << 8) | ((uint32_t)buf[3] << 16) | ((uint32_t)buf[4] << 24);

            return 0;
            break;
        
            //TOUCH EVENT (0x65)
        case 0x65:
            value = (uint32_t)buf[2];
            return value;
            break;
        case 0x66:
            value = (uint32_t)buf[1];
            
            break;
        default:
            break;
        }

        continue;
    }
}


int read_nextion_message(uint8_t *buf, int maxlen)
{
    int count = 0;
    uint8_t a,b,c;

    while (count < maxlen) {

        // Wait for first byte
        int ch = getchar_timeout(500);
        if (ch < 0) return 0;    // timeout = no message

        buf[count++] = ch;

        // If we have at least 3 bytes, check for FF FF FF
        if (count >= 3) {
            a = buf[count-3];
            b = buf[count-2];
            c = buf[count-1];

            if (a == 0xFF && b == 0xFF && c == 0xFF)
                return count;   // <-- full message received
        }
    }
    return count; // reached max length
}

//Black magic dont touch, GPT special
int getchar_timeout(uint16_t timeout_ms)
{
    while (timeout_ms--) {
        if (UCSR0A & (1 << RXC0))      // data available?
            return UDR0;               // return received byte
        _delay_ms(1);
    }
    return -1;                         // timeout
}

int get_page(){
    char valtype;
    printf("sendme%c%c%c",255,255,255);
    int val = read_nextion_value(&valtype);
    while(valtype != 0x66){
        printf("sendme%c%c%c",255,255,255);
        val = read_nextion_value(&valtype);
    }
    return val;

}