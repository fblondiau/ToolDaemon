#ifdef __cplusplus
extern "C" {
#endif

OSErr authenticateUser (StringPtr enteredUserNamePtr, StringPtr passwordPtr,
	StringPtr administratorKeyPtr, StringPtr foundUserNamePtr);

#ifdef __cplusplus
}
#endif
