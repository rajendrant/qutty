#ifndef STORAGE_H
#define STORAGE_H

/*
 * Read a saved session. The caller is expected to call
 * open_setting_r() to get a `void *' handle, then pass that to a
 * number of calls to read_setting_s() and read_setting_i(), and
 * then close it using close_settings_r().
 *
 * read_setting_s() writes into the provided buffer and returns a
 * pointer to the same buffer.
 *
 * If a particular string setting is not present in the session,
 * read_setting_s() can return NULL, in which case the caller
 * should invent a sensible default. If an integer setting is not
 * present, read_setting_i() returns its provided default.
 *
 * read_setting_filename() and read_setting_fontspec() each read into
 * the provided buffer, and return zero if they failed to.
 */
void *open_settings_r(const char *sessionname);
char *read_setting_s(void *handle, const char *key, char *buffer, int buflen);
int read_setting_i(void *handle, const char *key, int defvalue);
int read_setting_filename(void *handle, const char *key, Filename *value);
int read_setting_fontspec(void *handle, const char *key, FontSpec *font);
void close_settings_r(void *handle);

/*
 * Enumerate all saved sessions.
 */
void *enum_settings_start(void);
char *enum_settings_next(void *handle, char *buffer, int buflen);
void enum_settings_finish(void *handle);

/*
 * See if a host key matches the database entry. Return values can
 * be 0 (entry matches database), 1 (entry is absent in database),
 * or 2 (entry exists in database and is different).
 */
int verify_host_key(const char *hostname, int port,
            const char *keytype, const char *key);

/* ----------------------------------------------------------------------
 * Functions to access PuTTY's random number seed file.
 */

typedef void (*noise_consumer_t) (void *data, int len);

#endif // STORAGE_H
