#ifndef HTTP_H_
#define HTTP_H_

int http_post_audio_buff(String url, char* audio_data, uint32_t len, char* outfilename);
int http_post_audio_stream(String url, Stream * stream, uint32_t len, char* outfilename);
#endif /* HTTP_H_ */
