#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>
#include <ngx_http_variables.h>

#if (NGX_FREEBSD)
#error FreeBSD is not supported yet, sorry.
#elif (NGX_DARWIN)
#include <uuid.h>      
#elif (NGX_SOLARIS)
#error Solaris is not supported yet, sorry.
#elif (NGX_LINUX)
#include <uuid/uuid.h>      
#endif

// TODO:
//
// * make the name of the variable configurable

typedef struct {
    ngx_flag_t     enable;
    ngx_str_t      name;
} ngx_x_rid_header_conf_t;


static void * ngx_x_rid_header_main_conf(ngx_conf_t *cf);
static void * ngx_x_rid_header_create_conf(ngx_conf_t *cf);
static char *ngx_x_rid_header_merge_conf(ngx_conf_t *cf,
    void *parent, void *child);
static ngx_int_t ngx_x_rid_header_add_variables(ngx_conf_t *cf);    
static char *
ngx_x_rid_header_set_name(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_str_t  ngx_x_rid_header_variable_name = ngx_string("request_id");

static ngx_command_t ngx_x_rid_header_module_commands[] = {

    { ngx_string("uuid"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
                        |NGX_CONF_FLAG,
      ngx_conf_set_flag_slot,
      NGX_HTTP_LOC_CONF_OFFSET,
      offsetof(ngx_x_rid_header_conf_t,enable),
      NULL },
      
      { ngx_string("uuid_name"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_HTTP_LIF_CONF
                        |NGX_CONF_FLAG,
      ngx_x_rid_header_set_name,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },
      
      ngx_null_command
};
    
static ngx_http_module_t  ngx_x_rid_header_module_ctx = {
  ngx_x_rid_header_add_variables,     /* preconfiguration */
  NULL,                               /* postconfiguration */

  ngx_x_rid_header_main_conf,        /* create main configuration */
  NULL,        /* init main configuration */
            
  NULL,        /* create server configuration */
  NULL,        /* merge server configuration */
            
  ngx_x_rid_header_create_conf,             /* create location configuration */
  ngx_x_rid_header_merge_conf               /* merge location configuration */
};                        


                      
ngx_module_t  ngx_x_rid_header_module = {
  NGX_MODULE_V1,
  &ngx_x_rid_header_module_ctx,      /* module context */
  ngx_x_rid_header_module_commands,  /* module directives */
  NGX_HTTP_MODULE,                   /* module type */
  NULL,                              /* init master */              
  NULL,                              /* init module */              
  NULL,                              /* init process */             
  NULL,                              /* init thread */              
  NULL,                              /* exit thread */              
  NULL,                              /* exit process */             
  NULL,                              /* exit master */              
  NGX_MODULE_V1_PADDING   
};


ngx_int_t ngx_x_rid_header_get_variable(ngx_http_request_t *r, ngx_http_variable_value_t *v, uintptr_t data) {
    
     ngx_x_rid_header_conf_t *conf;
    conf = ngx_http_get_module_loc_conf(r, ngx_x_rid_header_module);
  if(!conf->enable) {
    return NGX_OK;  
  }
    
  u_char *p;     

  p = ngx_pnalloc(r->pool, 37);
  if (p == NULL) {
      return NGX_ERROR;
  }       
      
#if (NGX_FREEBSD)
#error FreeBSD is not supported yet, sorry.
#elif (NGX_DARWIN)
  uuid_t* uuid;
  if ( uuid_create(&uuid) ) {
    return -1;
  }
  if ( uuid_make(uuid, UUID_MAKE_V4) ) {
    uuid_destroy(uuid);
    return -1;
  }
  size_t data_len = 37;
  if ( uuid_export(uuid, UUID_FMT_STR, &p, &data_len) ) {
    uuid_destroy(uuid);
    return -1;
  }
  uuid_destroy(uuid);
#elif (NGX_SOLARIS)
#error Solaris is not supported yet, sorry.
#elif (NGX_LINUX)
  uuid_t uuid;
  uuid_generate(uuid);       
  uuid_unparse_lower(uuid, (char*)p);
#endif

  v->len = 36;
  v->valid = 1;
  v->no_cacheable = 0;
  v->not_found = 0;
  v->data = p;

  return NGX_OK;
}   
                                  


static ngx_int_t ngx_x_rid_header_add_variables(ngx_conf_t *cf)
{
     ngx_x_rid_header_conf_t *conf;
   conf = ngx_http_conf_get_module_main_conf(cf, ngx_x_rid_header_module);
   
  ngx_http_variable_t* var = ngx_http_add_variable(cf, &conf->name, NGX_HTTP_VAR_NOHASH);
  if (var == NULL) {
      return NGX_ERROR;
  }
  var->get_handler = ngx_x_rid_header_get_variable;
  return NGX_OK;
}

static void * ngx_x_rid_header_create_conf(ngx_conf_t *cf)
{
    ngx_x_rid_header_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_x_rid_header_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->enable = NGX_CONF_UNSET;
    //ngx_str_set(&conf->name, &ngx_x_rid_header_variable_name);
    
    return conf;
}

static char *
ngx_x_rid_header_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_x_rid_header_conf_t *prev = parent;
    ngx_x_rid_header_conf_t *conf = child;
    ngx_x_rid_header_conf_t *lcf;

    ngx_conf_merge_value(conf->enable, prev->enable, 0);
    
    lcf = ngx_http_conf_get_module_main_conf(cf, ngx_x_rid_header_module);
     ngx_str_set(&conf->name, &lcf->name);
     
    return NGX_CONF_OK;
}

               
static char *
ngx_x_rid_header_set_name(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_x_rid_header_conf_t *llcf = conf;

    ngx_str_t   *value;
    
    if (cf->cmd_type != NGX_HTTP_MAIN_CONF) {
        ngx_conf_log_error(NGX_LOG_WARN, cf, 0,
                           "the \"uuid_name\" directive may be used "
                           "only on \"http\" level");
    }

    if (llcf->name.len > 0) {
        return "is duplicate";
    }

    value = cf->args->elts;
    ngx_str_set(&llcf->name, &value[1]);
    return NGX_CONF_OK;
}

static void *
ngx_x_rid_header_main_conf(ngx_conf_t *cf)
{
    ngx_x_rid_header_conf_t  *conf;
    //conf->enable = 0;
    ngx_str_set(&conf->name, &ngx_x_rid_header_variable_name);
    return conf;
}


