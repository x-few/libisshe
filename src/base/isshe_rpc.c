#include "isshe_common.h"

isshe_rpc_client_t *
isshe_clnt_create(
    isshe_char_t *host, isshe_ulong_t prog,
    isshe_ulong_t vers, isshe_char_t *proto)
{
    isshe_rpc_client_t	*cl;

    if ( (cl = clnt_create(host, prog, vers, proto)) == NULL) {
        clnt_pcreateerror(host);
        isshe_error_exit("clnt_create error");
    }

    return(cl);
}

isshe_bool_t
isshe_clnt_control(isshe_rpc_client_t *cl,
    isshe_int_t req, isshe_char_t *info)
{
    isshe_bool_t ret;

    if ( (ret = clnt_control(cl, req, info)) == ISSHE_FALSE) {
        isshe_error_exit("clnt_control error");
    }

    return(ret);
}
