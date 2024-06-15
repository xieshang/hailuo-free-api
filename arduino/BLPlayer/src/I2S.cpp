#include "I2S.h"
#include <driver/i2s.h>

i2s_port_t I2S_READ_PORT;
i2s_port_t I2S_WRITE_PORT;

void I2S_Init(i2s_port_t I2S_NUM, i2s_mode_t MODE, i2s_bits_per_sample_t BPS, uint16_t dma_len)
{
  i2s_config_t i2s_config;
  if (MODE == I2S_MODE_RX) {
    i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = 44100,
      .bits_per_sample = i2s_bits_per_sample_t(16),
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = 0, // default interrupt priority
      .dma_buf_count = 8,
      .dma_buf_len = dma_len,
      .use_apll = false
    };
  }
  if (MODE == I2S_MODE_TX) {
    i2s_config = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 4410,
      .bits_per_sample = i2s_bits_per_sample_t(16),
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),
      .intr_alloc_flags = 0, // default interrupt priority
      .dma_buf_count = 8,
      .dma_buf_len = dma_len,
      .use_apll = false
    };
  }

  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);

  // i2s_config_t i2s_config = {
  //   .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_TX),
  //   .sample_rate = SAMPLE_RATE,
  //   .bits_per_sample = BPS,
  //   .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
  //   .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_STAND_I2S),//1111 (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
  //   .intr_alloc_flags = 0,
  //   .dma_buf_count = 8,
  //   .dma_buf_len = 60
  // };
  
  // i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);

  if (MODE == I2S_MODE_RX || MODE == I2S_MODE_TX) {
    Serial.printf("using I2S_MODE: %d\n", MODE);
    i2s_pin_config_t pin_config;
    pin_config.bck_io_num = PIN_I2S_BCLK;
    pin_config.ws_io_num = PIN_I2S_WS;
    pin_config.mck_io_num = -1;
    if (MODE == I2S_MODE_RX) {
      I2S_READ_PORT = I2S_NUM;
      pin_config.data_out_num = I2S_PIN_NO_CHANGE;
      pin_config.data_in_num = PIN_I2S_DIN;
    } 
    else if (MODE == I2S_MODE_TX) {
      I2S_WRITE_PORT = I2S_NUM;
      pin_config.data_out_num = PIN_I2S_DOUT;
      pin_config.data_in_num = I2S_PIN_NO_CHANGE;
    }

    i2s_set_pin(I2S_NUM, &pin_config);
    // i2s_set_clk(I2S_NUM_0, SAMPLE_RATE, BPS, I2S_CHANNEL_STEREO);
    i2s_start(I2S_NUM);
  }
}

int I2S_Read(char *data, int numData)
{
  size_t outlen = 0;
  i2s_read(I2S_READ_PORT, (char *)data, numData, &outlen, portMAX_DELAY);
  return outlen;
}

void I2S_Write(char *data, int numData)
{
    size_t bytes_written = 0;
  i2s_write(I2S_WRITE_PORT, (const char *)data, numData, &bytes_written, portMAX_DELAY);
}



