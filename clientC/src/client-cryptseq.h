#ifndef CLIENT_CRYPTSEQ_H
#define CLIENT_CRYPTSEQ_H

void cryptseq_en(char *text, char *new_text);
void cryptseq_de(char *encrypted, char *text);

void cryptassoc_en(char *text, char *ciphered);
void cryptassoc_de(char *text, char *deciphered);

#endif
