// NFC-FortuneTelling Machine
// MCU: XIAO RP2040

#include <SPI.h>
#include <MFRC522.h>
#include <Adafruit_NeoPixel.h>

#define RST_PIN        (26)
#define SS_PIN          (1)
#define LED_PIN         (0)
#define NUMPIXELS     (256)

#define MAX_BRIGHTNESS (32)

MFRC522 rfid(SS_PIN, RST_PIN);
Adafruit_NeoPixel pixels(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define LINE_NUM (16)
const byte data_uranai[2*LINE_NUM]   = {0x00, 0x00, 0xFC, 0x3F, 0xFC, 0x3F, 0x0C, 0x30, 0x0C, 0x30, 0x0C, 0x30, 0xFC, 0x3F, 0xFC, 0x3F, 0x00, 0x03, 0x00, 0x03, 0xFE, 0x03, 0xFE, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00};
const byte data_kyo[2*LINE_NUM]      = {0x00, 0x00, 0xFE, 0x7F, 0xFE, 0x7F, 0x06, 0x60, 0x06, 0x68, 0x16, 0x6C, 0x36, 0x66, 0x66, 0x63, 0xC6, 0x61, 0xC6, 0x61, 0xE6, 0x63, 0x76, 0x66, 0x36, 0x6C, 0x36, 0x60, 0x06, 0x60, 0x00, 0x00};
const byte data_suekichi[2*LINE_NUM] = {0xF8, 0x1F, 0x18, 0x18, 0xF8, 0x1F, 0x00, 0x00, 0xFC, 0x3F, 0x80, 0x01, 0xFE, 0x7F, 0x80, 0x01, 0x00, 0x00, 0x86, 0x61, 0xB8, 0x1D, 0xC0, 0x03, 0xFC, 0x3F, 0x80, 0x01, 0xFE, 0x7F, 0x80, 0x01};
const byte data_kichi[2*LINE_NUM]    = {0x00, 0x00, 0xF8, 0x1F, 0xF8, 0x1F, 0x18, 0x18, 0x18, 0x18, 0xF8, 0x1F, 0xF8, 0x1F, 0x00, 0x00, 0xFC, 0x3F, 0xFC, 0x3F, 0x80, 0x01, 0xFE, 0x7F, 0xFE, 0x7F, 0x80, 0x01, 0x80, 0x01, 0x00, 0x00};
const byte data_shokichi[2*LINE_NUM] = {0xF8, 0x1F, 0x18, 0x18, 0xF8, 0x1F, 0x00, 0x00, 0xFC, 0x3F, 0x80, 0x01, 0xFE, 0x7F, 0x80, 0x01, 0x00, 0x00, 0x80, 0x03, 0x84, 0x21, 0x8C, 0x31, 0x98, 0x19, 0x98, 0x19, 0x80, 0x01, 0x80, 0x01};
const byte data_chukichi[2*LINE_NUM] = {0xF8, 0x1F, 0x18, 0x18, 0xF8, 0x1F, 0x00, 0x00, 0xFC, 0x3F, 0x80, 0x01, 0xFE, 0x7F, 0x80, 0x01, 0x00, 0x00, 0x80, 0x01, 0x80, 0x01, 0xFC, 0x3F, 0x84, 0x21, 0xFC, 0x3F, 0x80, 0x01, 0x80, 0x01};
const byte data_daikichi[2*LINE_NUM] = {0xF8, 0x1F, 0x18, 0x18, 0xF8, 0x1F, 0x00, 0x00, 0xFC, 0x3F, 0x80, 0x01, 0xFE, 0x7F, 0x80, 0x01, 0x00, 0x00, 0x06, 0x60, 0x1C, 0x1C, 0x60, 0x06, 0xC0, 0x03, 0xFE, 0x7F, 0x80, 0x01, 0x80, 0x01};

#define ID_MAX   (10)
const byte* datasets[ID_MAX] = {data_kyo, data_kyo, data_suekichi, data_kichi, data_shokichi, data_chukichi, data_chukichi, data_daikichi, data_daikichi, data_daikichi};

void setup()
{
    SPI.begin();
    rfid.PCD_Init();
    pixels.begin();
}

void loop()
{
    showLEDs(data_uranai, 0, getRainbowColor(false));
    delay(20);

    int32_t id = getRFID_ID();

    if (0 <= id)
    {
        do
        {
            byte data_no = 9 - (getSum(id) % 10);

            showEffect();
            showResult(data_no);
    
            for (int i=0; i<100; i++)
            {
                id = getRFID_ID();
                if (0 < id)
                {
                    break;
                }
                delay(100);
            }
        } while(0 < id);

        getRainbowColor(true); // reset color
    }
}

int getRFID_ID(void)
{
    if ( !rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial())
    {
        return -1;
    }

#if 1
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

    // Check is the PICC of Classic MIFARE type
    if( piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K   &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K   &&
        piccType != MFRC522::PICC_TYPE_MIFARE_UL)
    {
        return -1;
    }
#endif

    uint32_t id = 0;
    for (byte i = 0; i < 4; i++)
    {
        id = (256 * id) + rfid.uid.uidByte[i];
    }

    // Halt PICC
    rfid.PICC_HaltA();

    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();

    return id;
}

// SUM値の取得（val=4Byte分のみ）
int getSum(uint32_t val)
{
    int sum = 0;
    sum  = (int)(val >> 24) % 256;
    sum += (int)(val >> 16) % 256;
    sum += (int)(val >>  8) % 256;
    sum += (int)(val      ) % 256;

    return sum;
}

// おみくじの結果を表示
void showResult(uint32_t no)
{
    for (int i=0; i<17; i++)
    {
        showLEDs(datasets[no], i-16, pixels.Color(MAX_BRIGHTNESS, MAX_BRIGHTNESS, MAX_BRIGHTNESS));
        delay(250);
    }
}

// 画面切り替えエフェクト表示
void showEffect(void)
{
    static int effect     = 0;
    static int off_effect = 0;

    switch (effect)
    {
        case 0 : showEffect_A(); break;
        case 1 : showEffect_B(); break;
        case 2 : showEffect_C(); break;
        default: showEffect_D(); break;
    }

    switch (off_effect)
    {
        case 0 : showOffEffect_A(); break;
        case 1 : showOffEffect_B(); break;
        default: showOffEffect_C(); break;
    }

    effect     = (    effect + 1) % 4;
    off_effect = (off_effect + 1) % 3;
}

// 塗りつぶしエフェクトA（3連上昇）
void showEffect_A(void)
{
    uint32_t color = getRainbowColor(false);

    for (int i=0; i<16; i++)
    {
        SetLineColor(i, 0xFF, 0xFF, color);
        pixels.show();
        delay(20);
    }
    for (int j=0; j<MAX_BRIGHTNESS; j++) color = getRainbowColor(false);
    for (int i=0; i<16; i++)
    {
        SetLineColor(i, 0xFF, 0xFF, color);
        pixels.show();
        delay(20);
    }
    for (int j=0; j<MAX_BRIGHTNESS; j++) color = getRainbowColor(false);
    for (int i=0; i<16; i++)
    {
        SetLineColor(i, 0xFF, 0xFF, color);
        pixels.show();
        delay(20);
    }
}

// 塗りつぶしエフェクトB（一刀両断）
void showEffect_B(void)
{
    pixels.clear();
    for (int i=0; i<16; i++)
    {
        SetPixelColor(15-i, 15-i, pixels.Color(MAX_BRIGHTNESS,MAX_BRIGHTNESS,MAX_BRIGHTNESS));
        pixels.show();
        delay(10);
    }
    for (int i=1; i<MAX_BRIGHTNESS; i*=2)
    {
        for (int x=0; x<16; x++) for (int y=0; y<16; y++)
        {
            if (x != y)
            {
                SetPixelColor(x, y, pixels.Color(i,0,0));
            }
        }
        pixels.show();
        delay(200);
    }
}

// 塗りつぶしエフェクトC（中央から広がる）
void showEffect_C(void)
{
    uint32_t color = getRainbowColor(false);

    for (int i=0; i<8; i++)
    {
        for (int x=0; x<8; x++) for (int y=0; y<8; y++)
        {
            if (((7-x) <= i) && ((7-y) <= i))
            {
                SetPixelColor(   x,    y, color);
                SetPixelColor(15-x,    y, color);
                SetPixelColor(   x, 15-y, color);
                SetPixelColor(15-x, 15-y, color);
            }
        }
        pixels.show();
        delay(100);
    }
}

// 塗りつぶしエフェクトD（虹色）
void showEffect_D(void)
{
    uint32_t color = getRainbowColor(false);

    for (int i=0; i<24; i++)
    {
        for (int x=0; x<8; x++)
        {
            SetPixelColor(7-x, i-x, color);
            SetPixelColor(8+x, i-x, color);
        }

        pixels.show();
        delay(50);
        for (int j=0; j<8; j++) color = getRainbowColor(false);
    }
}

// 暗転エフェクトA（幕が開く）
void showOffEffect_A(void)
{
    byte x_right = 7;
    byte x_left  = 8;
    for (int i=0; i<8; i++)
    {
        for (int y=0; y<16; y++)
        {
            SetPixelColor(x_right, y, 0);
            SetPixelColor(x_left , y, 0);
        }
        pixels.show();
        delay(100);
        x_right--;
        x_left++;
    }
}

// 暗転エフェクトB（下に切り裂く）
void showOffEffect_B(void)
{
    for (int i=0; i<24; i++)
    {
        for (int x=0; x<8; x++)
        {
            SetPixelColor(7-x, 16-i+x, 0);
            SetPixelColor(8+x, 16-i+x, 0);
        }

        pixels.show();
        delay(50);
    }
}


// 暗転エフェクトC（１ピクセルずつ外へ）
void showOffEffect_C(void)
{
    for (int i=0; i<128; i++)
    {
        pixels.setPixelColor(127-i, 0);
        pixels.setPixelColor(128+i, 0);

        pixels.show();
        delay(5);
    }
}

// コールする度に変化する色の値を取得
uint32_t getRainbowColor(bool isReset)
{
    static byte red   = 0;
    static byte green = 0;
    static byte blue  = 0;

    if (isReset)
    {
        red   = 0;
        green = 0;
        blue  = 0;
    }
    else if ((red == green) && (green == blue) && (red < MAX_BRIGHTNESS))
    {
        red++;
        green++;
        blue++;
    }
    else if (red == MAX_BRIGHTNESS)
    {
        if (0 < blue)
        {
            blue--;
        }
        else if (green < MAX_BRIGHTNESS)
        {
            green++;
        }
        else
        {
            red--;
        }
    }
    else if (green == MAX_BRIGHTNESS)
    {
        if (0 < red)
        {
            red--;
        }
        else if (blue < MAX_BRIGHTNESS)
        {
            blue++;
        }
        else
        {
            green--;
        }
    }
    else if (blue == MAX_BRIGHTNESS)
    {
        if (0 < green)
        {
            green--;
        }
        else if (red < MAX_BRIGHTNESS)
        {
            red++;
        }
        else
        {
            blue--;
        }
    }
    else
    {
        /* error? */
    }

    return pixels.Color(red, green, blue);
}

// データを基に光らせる
void showLEDs(const byte data[], int32_t offset, const uint32_t color)
{
    pixels.clear();

    for(int y=0; y<16; y++)
    {
        SetLineColor(y, getDataRight(data, y, offset), getDataLeft(data, y, offset), color);
    }

    pixels.show();
}


// 2Byte(16pixel)のデータを基に光らせる
void SetLineColor(const uint32_t y, const byte data_right, const byte data_left, const uint32_t color)
{
    byte mask = 0x01;
    for(int j=0; j<8; j++)
    {
        if ((data_right & mask) != 0x00)
        {
            SetPixelColor(j, y, color);
        }
        if ((data_left & mask) != 0x00)
        {
            SetPixelColor(j+8, y, color);
        }

        mask *= 2;
    }
}


// LEDパネル右下を原点とし、左方向をX軸、上方向をY軸とした座標系に変換する
void SetPixelColor(const int32_t x, const int32_t y, const uint32_t color)
{
    if ((0 <= x) && (x < 16) && (0 <= y) && (y < 16))
    {
        uint32_t pix_no = (x * 16) + ((x%2)?(15 - y):y);
        pixels.setPixelColor(pix_no, color);
    }
}


// 指定Y座標、指定オフセットのデータ（右側）を取得
byte getDataRight(const byte data[], int32_t y, int32_t offset)
{
    if (((y + offset) < 0) ||
        (LINE_NUM <= (y + offset)))
    {
        return 0;
    }

    return data[(y + offset)*2];
}


// 指定Y座標、指定オフセットのデータ（左側）を取得
byte getDataLeft(const byte data[], int32_t y, int32_t offset)
{
    if (((y + offset) < 0) ||
        (LINE_NUM <= (y + offset)))
    {
        return 0;
    }

    return data[(y + offset)*2 + 1];
}
