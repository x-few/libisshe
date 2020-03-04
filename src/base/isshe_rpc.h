#ifndef _ISSHE_RPC_H_
#define _ISSHE_RPC_H_

#include "isshe_common.h"

typedef struct CLIENT isshe_rpc_client_t;


isshe_rpc_client_t *isshe_clnt_create(isshe_char_t *host,
    isshe_ulong_t prog, isshe_ulong_t vers, isshe_char_t *proto);

bool_t isshe_clnt_control(isshe_rpc_client_t *cl,
    isshe_int_t req, isshe_char_t *info);

#endif