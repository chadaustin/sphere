#include "unix_audio.h"

ADR_STREAM AdrOpenStream(ADR_CONTEXT context, const char* name) {
  static int a;
  return &a; }
int AdrCloseStream(ADR_STREAM stream) {
  return 0; }
void AdrSetStreamRepeat(ADR_STREAM stream, ADR_BOOL repeat) {}
void AdrPlayStream(ADR_STREAM stream) {}
int AdrPauseStream(...) {
  return 0; }
int AdrResetStream(...) {
  return 0; }
int AdrSetStreamVolume(...) {
  return 0; }
int AdrGetStreamVolume(...) {
  return 0; }
int AdrSetStreamPan(...) {
  return 0; }
int AdrGetStreamPan(...) {
  return 0; }

void SA_PushFileSystem (IFileSystem* fs) {
}

void SA_PopFileSystem () {
}

ADR_CONTEXT SA_GetAudiereContext () {
  static int a;

  return &a;
}
