#ifndef _klish_ktp_session_h
#define _klish_ktp_session_h

#include <faux/faux.h>
#include <faux/list.h>
#include <faux/eloop.h>
#include <klish/ksession.h>
#include <klish/ktp.h>

#define USOCK_PATH_MAX sizeof(((struct sockaddr_un *)0)->sun_path)

#define KLISH_DEFAULT_UNIX_SOCKET_PATH "/tmp/klish-unix-socket"

typedef struct ktpd_session_s ktpd_session_t;
typedef struct ktp_session_s ktp_session_t;

typedef bool_t (*ktpd_session_stall_cb_fn)(ktpd_session_t *session,
	void *user_data);

C_DECL_BEGIN

// Common KTP functions
int ktp_connect_unix(const char *sun_path);
void ktp_disconnect(int fd);
int ktp_accept(int listen_sock);

bool_t ktp_check_header(faux_hdr_t *hdr);
faux_msg_t *ktp_msg_preform(ktp_cmd_e cmd, uint32_t status);
bool_t ktp_send_error(faux_async_t *async, ktp_cmd_e cmd, const char *error);

bool_t ktp_peer_ev(faux_eloop_t *eloop, faux_eloop_type_e type,
	void *associated_data, void *user_data);
bool_t ktp_stall_cb(faux_async_t *async, size_t len, void *user_data);


// Client KTP session
ktp_session_t *ktp_session_new(int sock);
void ktp_session_free(ktp_session_t *session);
bool_t ktp_session_done(const ktp_session_t *ktp);
bool_t ktp_session_set_done(ktp_session_t *ktp, bool_t done);
bool_t ktp_session_connected(ktp_session_t *session);
int ktp_session_fd(const ktp_session_t *session);
bool_t ktp_session_req_cmd(ktp_session_t *ktp, const char *line, int *retcode);


// Server KTP session
ktpd_session_t *ktpd_session_new(int sock, const kscheme_t *scheme,
	const char *start_entry, faux_eloop_t *eloop);
void ktpd_session_free(ktpd_session_t *session);
bool_t ktpd_session_connected(ktpd_session_t *session);
int ktpd_session_fd(const ktpd_session_t *session);
bool_t ktpd_session_async_in(ktpd_session_t *session);
bool_t ktpd_session_async_out(ktpd_session_t *session);

C_DECL_END

#endif // _klish_ktp_session_h
