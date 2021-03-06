#ifndef _uri_parser_
#define _uri_parser_
#ifdef  __cplusplus
extern "C" {
#endif
typedef struct {
	char *scheme;               /* mandatory */
	char *host;                 /* mandatory */
	char *port;                 /* optional */
	char *path;                 /* optional */
	char *query;                /* optional */
	char *fragment;             /* optional */
	char *username;             /* optional */
	char *password;             /* optional */
    char *extension;
    char *host_ext;
	char *_uri;             	/* private */
	int _uri_len;             	/* private */
} parsed_uri_t;

parsed_uri_t *parse_uri(char *);
void free_parsed_uri(parsed_uri_t *);
void parse_uri_info(parsed_uri_t *puri);
#ifdef __cplusplus
}
#endif

#endif /* _uri_parser_ */
