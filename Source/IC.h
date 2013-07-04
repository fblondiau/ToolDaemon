#ifdef __cplusplus
extern "C" {
#endif

Boolean checkICStart (void);
Boolean checkICStop (void);

Boolean canUseIC (void);
void checkICLaunchURL (StringPtr URL);
StringPtr checkICGetKey (StringPtr result, ConstStr255Param key);

#ifdef __cplusplus
}
#endif
