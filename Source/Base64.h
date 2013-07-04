#ifdef __cplusplus
extern "C" {
#endif

/* encoded len must be (at least) 4/3 of s len */
unsigned char * inBase64 (unsigned char * encoded, const unsigned char * s);

/* decoded len must be (at least) 3/4 of s len */
unsigned char * outBase64 (unsigned char * decoded, const unsigned char * s);

#ifdef __cplusplus
}
#endif
