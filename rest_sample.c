/*
 * Functions: 
 * This sample code is showing how to access the REST APIs of the other systems 
 * with methods including GET, PUT, POST and DELETE. The access need the 
 * authentication information of the accessed systems. it also shows how to use 
 * the open source tool cJSON to format the JSON data as an object to use.
 *
 * Tips:
 * Because this sample code is accessing the REST API of the other system, it is
 * related to the system information of the accessed system. So in oder to run 
 * this sample code, you need to change the host, username, password and
 * port number as your system information. You also need to pay attenation to
 * other information related to the REST API e.g. certification_path, protocol, 
 * url, request_body and so on. Please make sure they are consistent with your 
 * system.
 *
 */

#include <stdio.h>
#include <axis2_http_request_line.h>
#include <axis2_http_status_line.h>
#include <axis2_http_header.h>
#include <axutil_error_default.h>
#include <axutil_url.h>
#include <axis2_http_client.h>
#include <axutil_stream.h>
#include <axutil_base64.h>
#include <cJSON.h>

typedef struct a
{
    axis2_char_t *value;
}

a;

/*初始化环境*/
axutil_env_t *test_init()
{
    axutil_allocator_t *allocator = axutil_allocator_init(NULL);
    axutil_error_t *error = axutil_error_create(allocator);
    axutil_env_t *env = NULL;
    env = axutil_env_create_all("rest_sample.log", AXIS2_LOG_LEVEL_TRACE);
    return env;
}

/*初始化request_line*/
axis2_http_request_line_t *test_request_line_init(const axutil_env_t * env, axis2_char_t * method, axis2_char_t * path, axis2_char_t * http_version)
{
    axis2_http_request_line_t *request_line = NULL;
    request_line = axis2_http_request_line_create(env, method, path, http_version);
    return request_line;
}

/*初始化url*/
axutil_url_t *test_url_init(const axutil_env_t * env, axis2_char_t * protocol, axis2_char_t * host, int port,axis2_char_t * path)
{
    axutil_url_t *url = NULL;
    url = axutil_url_create(env, protocol, host, port, path);
    return url;
}

/*用于查找字符在字符串中的位置*/
int indexOf(char *str1,char *str2)  
{  
    char *p=str1;  
    int i=0;  
    p=strstr(str1,str2);  
    if(p==NULL)  
        return -1;  
    else{  
        while(str1!=p)  
        {  
            str1++;  
            i++;  
        }  
    }  
    return i;  
}  

/*反向查找字符在字符串中的位置*/
int lastIndexOf(char *str1,char *str2)  
{  
    char *p=str1;  
    int i=0,len=strlen(str2);  
    p=strstr(str1,str2);  
    if(p==NULL)return -1;  
    while(p!=NULL)  
    {  
        for(;str1!=p;str1++)i++;  
        p=p+len;  
        p=strstr(p,str2);  
    }  
    return i;  
}

/*合成username:password形式字符串*/
char * form_user_pass_str(char *username, char *password)
{
    int user_size = strlen(username);
    int pass_size = strlen(password);
    int str_formed_size = user_size + pass_size + 1;
    char *str_formed = (char *) malloc((str_formed_size) * sizeof(char));
    strcpy(str_formed, username);
    strcat(str_formed, ":");
    strcat(str_formed, password);
    return str_formed;
}

/*获得Authentication字符串*/
char *get_Authentication_Str(const axutil_env_t * env,axis2_char_t *str_encode)
{
    int str_size = 0;
    int encoded_len = 0;
    axis2_char_t *str_src = NULL;
    axis2_char_t *encoded_str = NULL;
    str_src = str_encode;
    str_size = strlen(str_src);
    encoded_len = axutil_base64_encode_len(str_size);
    encoded_str = AXIS2_MALLOC(env->allocator, encoded_len + 2);
    encoded_len = axutil_base64_encode(encoded_str, str_src, str_size);
    encoded_str[encoded_len] = '\0';
    axis2_char_t *firstStr = NULL;
    firstStr = "Basic";
    int size = 0;
    int firstStrSize = strlen(firstStr);
    int encoded_strSize = strlen(encoded_str);
    if(firstStrSize > encoded_strSize){
        size = ((int)strlen(firstStr)) * 2;
    }else{
        size = ((int)strlen(encoded_str)) * 2;
    }
    char *str_return = (char *) malloc((size + 2) * sizeof(char));
    strcpy(str_return, firstStr);
    strcat(str_return, " ");
    strcat(str_return, encoded_str);
    return str_return;
}

/*GET方法的实现*/
void test_rest_get(const axutil_env_t * env, axis2_http_request_line_t *request_line,axutil_url_t *url, axis2_char_t *username, 
					axis2_char_t *password, axis2_char_t *certification_path)
{

    printf("Starting the REST GET test!\n");
    axis2_http_client_t *client = NULL;
    axis2_http_simple_request_t *request = NULL;
    axis2_http_header_t *header = NULL;
    axutil_stream_t *request_body = NULL;
    axis2_http_simple_response_t *response = NULL;
    int status = 0;
    char *body_bytes = NULL;
    int body_bytes_len = 0;

    /*创建请求消息*/
    request_body = axutil_stream_create_basic(env);
    request = axis2_http_simple_request_create(env, request_line,NULL, 0, NULL);

    /*添加消息头*/
    header = axis2_http_header_create(env, "Host", axutil_url_get_host(url, env));
    axis2_http_simple_request_add_header(request, env, header);

    /*添加用户身份验证*/
    char *user_pass_str = NULL;
    char *str_return = NULL;
    user_pass_str = form_user_pass_str(username, password);
    str_return = get_Authentication_Str(env, user_pass_str);
    header = axis2_http_header_create(env, "Authorization", str_return);
    axis2_http_simple_request_add_header(request, env, header);

    client = axis2_http_client_create(env, url);
    status = axis2_http_client_set_server_cert(client, env, certification_path);

    /*发送请求*/
    status = axis2_http_client_send(client, env, request, NULL);
    if (status < 0)
    {
        printf("Test FAILED .........Can't send the request. Status :%d\n", status);
		return;
    }
    status = axis2_http_client_recieve_header(client, env);
    if (status < 0)
    {
        printf("Test FAILED ......... Can't recieve. Status: %d\n", status);
        return;
    }
	
    /*接收请求*/
    response = axis2_http_client_get_response(client, env);
    if (!response)
    {
        printf("Test Failed : NULL response");
        return;
    }
    printf("Content Type :%s\n", axis2_http_simple_response_get_content_type(response, env));
    printf("Content Length :%d\n",
    axis2_http_simple_response_get_content_length(response, env));
    printf("Status code :%d\n", status);

    /*得到返回结果字符串及其长度*/
    body_bytes_len =
    axis2_http_simple_response_get_body_bytes(response, env, &body_bytes);
    printf("The response body length is : %d\n", body_bytes_len);
    printf("The response body is : %s\n", body_bytes);
    
    /*处理得到的返回结果body_bytes*/
    char *body_bytes_more = NULL;
    int body_bytes_len_more = axis2_http_simple_response_get_body_bytes(response, env, &body_bytes_more);
    int temp_size = 0;
    int resp_Str = 0;
    int resp_Str_More = 0;
    char *str_end = NULL;

    /*只要response中还存在内容，就做循环读取*/
    while(body_bytes_len_more != 0){
        resp_Str = strlen(body_bytes);
        resp_Str_More = strlen(body_bytes_more);
        if(resp_Str > resp_Str_More){
          temp_size = ((int)strlen(body_bytes)) * 2;
        }else{
          temp_size = ((int)strlen(body_bytes_more)) * 2;
        }
        str_end = (char *) malloc((temp_size + 2) * sizeof(char));
        strcpy(str_end, body_bytes);
        strcat(str_end, body_bytes_more);
        body_bytes_len = body_bytes_len + body_bytes_len_more;
        body_bytes_len_more = axis2_http_simple_response_get_body_bytes(response, env, &body_bytes_more);
    }

    /*处理JSON格式字符串*/
    int startIndex; 
    int endIndex;
    int strLength;
    char *startChar = NULL;
    char *endChar = NULL;
    startChar = "{"; 
    endChar = "}";
    startIndex = indexOf(str_end, startChar);
    endIndex = lastIndexOf(str_end, endChar);
    strLength = endIndex - startIndex;
    char document[strLength];
    int i = startIndex;
    while(i<=endIndex){
        document[i-startIndex] = str_end[i];
        i++;
    }
    document[i-startIndex] = "\0";

    char *result = NULL;

    cJSON *root;
    cJSON *label;
    root = cJSON_Parse(document);

    result = cJSON_Print(root);
    printf("The format response data is : %s\n", result);

    cJSON *resources = cJSON_GetObjectItem(root, "resources");
    int item_size = cJSON_GetArraySize(resources);    
    printf("\nThe resources has %d items.\n", item_size);

    label = cJSON_GetArrayItem(resources, item_size-1);
    result = cJSON_Print(label);
    printf("\nThe item %d is : %s\n", (item_size-1), result);
    
    /*释放空间*/
    axis2_http_client_free(client, env);
    axis2_http_simple_request_free(request, env);
    axutil_stream_free(request_body, env);
    AXIS2_FREE(env->allocator, body_bytes);
    printf("Finished the REST GET test!\n\n\n");
}

/*POST方法的实现*/
void test_rest_post(const axutil_env_t * env, axis2_http_request_line_t *request_line, axutil_url_t *url, axis2_char_t *username,
					 axis2_char_t *password, axis2_char_t *certification_path, axis2_char_t *body_request){

    axis2_http_client_t *client = NULL;
    axis2_http_simple_request_t *request = NULL;
    axis2_http_header_t *header = NULL;
    axutil_stream_t *request_body = NULL;
    axis2_http_simple_response_t *response = NULL;
    int status = 0;
    char *body_bytes = NULL;
    int body_bytes_len = 0;

    printf("Starting the REST POST test!\n");
    request_body = axutil_stream_create_basic(env);

    /*添加requestbody字符串*/
    int body_request_len = 0;
    body_request_len = strlen(body_request);
    axutil_stream_write(request_body ,env, body_request, body_request_len);
    request = axis2_http_simple_request_create(env, request_line, NULL, 0, request_body);
    
    /*添加消息头*/
    header = axis2_http_header_create(env, "Host", axutil_url_get_host(url, env));
    axis2_http_simple_request_add_header(request, env, header);

    /*添加用户身份验证*/
    char *user_pass_str = NULL;
    char *str_return = NULL;
    user_pass_str = form_user_pass_str(username, password);
    str_return = get_Authentication_Str(env, user_pass_str);
    header = axis2_http_header_create(env, "Authorization", str_return);
    axis2_http_simple_request_add_header(request, env, header);

    /*添加消息头*/
    header =axis2_http_header_create(env, "Content-Type", "application/json");
    axis2_http_simple_request_add_header(request, env, header);

    int request_body_value = axutil_stream_get_len(request_body,env);
    char content_value[request_body_value];
    sprintf(content_value, "%d", request_body_value);	

    header = axis2_http_header_create(env, "Content-Length", content_value);
    axis2_http_simple_request_add_header(request, env, header);

    client = axis2_http_client_create(env, url);
    status = axis2_http_client_set_server_cert(client, env, certification_path);

    /*发送请求*/
    status = axis2_http_client_send(client, env, request, NULL);
    if (status < 0)
    {
        printf("Test FAILED .........Can't send the request. Status :%d\n", status);
        return;
    }
    status = axis2_http_client_recieve_header(client, env);
    if (status < 0)
    {
        printf("Test FAILED ......... Can't recieve. Status: %d\n", status);
        return;
    }

    /*接收请求*/
    response = axis2_http_client_get_response(client, env);
    if (!response)
    {
        printf("Test Failed : NULL response");
        return;
    }
    printf("Content Type :%s\n", axis2_http_simple_response_get_content_type(response, env));
    printf("Content Length :%d\n", axis2_http_simple_response_get_content_length(response, env));
    printf("Status code :%d\n", status);

    /*得到返回结果字符串及其长度*/
    body_bytes_len = axis2_http_simple_response_get_body_bytes(response, env, &body_bytes);
    printf("The response body length is : %d\n", body_bytes_len);
    printf("The response body is : %s\n", body_bytes);

    /*释放空间*/
    axis2_http_client_free(client, env);
    axis2_http_simple_request_free(request, env);
    axutil_stream_free(request_body, env);
    AXIS2_FREE(env->allocator, body_bytes);
    printf("Finished the REST POST test!\n\n\n");
}

/*PUT方法的实现*/
void test_rest_put(const axutil_env_t * env, axis2_http_request_line_t *request_line, axutil_url_t *url,axis2_char_t *username, 
					axis2_char_t *password, axis2_char_t *certification_path, axis2_char_t *body_request){

    axis2_http_client_t *client = NULL;
    axis2_http_simple_request_t *request = NULL;
    axis2_http_header_t *header = NULL;
    axutil_stream_t *request_body = NULL;
    axis2_http_simple_response_t *response = NULL;
    int status = 0;
    char *body_bytes = NULL;
    int body_bytes_len = 0;

    printf("Starting REST PUT test!\n");

    /*添加requestbody字符串*/
    request_body = axutil_stream_create_basic(env);
    int body_request_len = 0;
    body_request_len = strlen(body_request);
    axutil_stream_write(request_body, env, body_request, body_request_len);
    request = axis2_http_simple_request_create(env, request_line, NULL, 0, request_body);

    header = axis2_http_header_create(env, "Host", axutil_url_get_host(url, env));
    axis2_http_simple_request_add_header(request, env, header);

    /*添加用户身份验证*/
    char *user_pass_str = NULL;
    char *str_return = NULL;
    user_pass_str = form_user_pass_str(username, password);
    str_return = get_Authentication_Str(env, user_pass_str);
    header = axis2_http_header_create(env, "Authorization", str_return);
    axis2_http_simple_request_add_header(request, env, header);

    /*添加消息头*/
    header = axis2_http_header_create(env, "Content-Type", "application/json");
    axis2_http_simple_request_add_header(request, env, header);

    int request_body_value = axutil_stream_get_len(request_body, env);
    char content_value[request_body_value];
    sprintf(content_value, "%d", request_body_value);

    header = axis2_http_header_create(env, "Content-Length", content_value);
    axis2_http_simple_request_add_header(request, env, header);

    client = axis2_http_client_create(env, url);
    status = axis2_http_client_set_server_cert(client, env, certification_path);

    /*发送请求*/
    status = axis2_http_client_send(client, env, request, NULL);
    if (status < 0)
    {
        printf("Test FAILED .........Can't send the request. Status :%d\n", status);
        return;
    }
    status = axis2_http_client_recieve_header(client, env);
    if (status < 0)
    {
        printf("Test FAILED ......... Can't recieve. Status: %d\n", status);
        return;
    }

    /*接收请求*/
    response = axis2_http_client_get_response(client, env);
    if (!response)
    {
        printf("Test Failed : NULL response");
        return;
    }
    printf("Content Type :%s\n",axis2_http_simple_response_get_content_type(response, env));
    printf("Content Length :%d\n",
    axis2_http_simple_response_get_content_length(response, env));
    printf("Status code :%d\n", status);

    /*获得响应字符串*/
    body_bytes_len =
    axis2_http_simple_response_get_body_bytes(response, env, &body_bytes);
    printf("The response body length is : %d\n", body_bytes_len);
    printf("The response body is : %s\n", body_bytes);

    /*释放空间*/
    axis2_http_client_free(client, env);
    axis2_http_simple_request_free(request, env);
    axutil_stream_free(request_body, env);
    AXIS2_FREE(env->allocator, body_bytes);
    printf("Finished the REST PUT test!\n\n\n");
}

/*DELETE方法的实现*/
void test_rest_delete(const axutil_env_t * env, axis2_http_request_line_t *request_line, axutil_url_t *url, axis2_char_t *username, 
						axis2_char_t *password, axis2_char_t *certification_path){

    axis2_http_client_t *client = NULL;
    axis2_http_simple_request_t *request = NULL;
    axis2_http_header_t *header = NULL;
    axutil_stream_t *request_body = NULL;
    axis2_http_simple_response_t *response = NULL;
    int status = 0;
    char *body_bytes = NULL;
    int body_bytes_len = 0;

    printf("Starting REST DELETE test！\n");

    request_body = axutil_stream_create_basic(env);
    request = axis2_http_simple_request_create(env, request_line, NULL, 0, NULL);

    /*添加消息头*/
    header = axis2_http_header_create(env, "Host", axutil_url_get_host(url, env));
    axis2_http_simple_request_add_header(request, env, header);

    /*添加用户身份验证*/
    char *user_pass_str = NULL;
    char *str_return = NULL;
    user_pass_str = form_user_pass_str(username, password);
    str_return = get_Authentication_Str(env, user_pass_str);
    header = axis2_http_header_create(env, "Authorization", str_return);
    axis2_http_simple_request_add_header(request, env, header);

    client = axis2_http_client_create(env, url);
    status = axis2_http_client_set_server_cert(client, env, certification_path);

    /*发送请求*/
    status = axis2_http_client_send(client, env, request, NULL);
    if (status < 0)
    {
		printf("Test FAILED .........Can't send the request. Status :%d\n", status);
		return;
    }
    status = axis2_http_client_recieve_header(client, env);
    if (status < 0)
    {
        printf("Test FAILED ......... Can't recieve. Status: %d\n", status);
        return;
    }

    /*接收请求*/
    response = axis2_http_client_get_response(client, env);
    if (!response)
    {
        printf("Test Failed : NULL response");
        return;
    }
    printf("Content Type :%s\n", axis2_http_simple_response_get_content_type(response, env));
    printf("Content Length :%d\n",
    axis2_http_simple_response_get_content_length(response, env));
    printf("Status code :%d\n", status);

    /*处理响应消息*/
    body_bytes_len =
    axis2_http_simple_response_get_body_bytes(response, env, &body_bytes);
    printf("The response body length is : %d\n", body_bytes_len);
    printf("The response body is : %s\n", body_bytes);

    /*释放空间*/
    axis2_http_client_free(client, env);
    axis2_http_simple_request_free(request, env);
    axutil_stream_free(request_body, env);
    AXIS2_FREE(env->allocator, body_bytes);
    printf("Finished the REST DELETE test!\n\n\n");
}

/*测试主函数*******/
int main(void)
{
    axutil_env_t *env = test_init();

    /*设置测试用例参数*/
    axis2_char_t * http_version = "HTTP/1.1";
    axis2_char_t * host_address = "10.11.12.13";
    axis2_char_t * username = "username";
    axis2_char_t * password = "password";
    axis2_char_t * certification_path = "/home/cert.pem";
    axis2_char_t * protocol = "https";
    int port = 8080;

    axis2_char_t * method = NULL;
    axis2_char_t * path = NULL;
    axis2_char_t * request_body = NULL;
    
    axis2_http_request_line_t *request_line = NULL;
    axutil_url_t *url = NULL;

    /*GET方法测试*/
    method = "GET";
    path = "/ibm/director/rest/resources/Server";

    request_line = test_request_line_init(env, method, path, http_version);
    url = test_url_init(env, protocol, host_address, port, path);
    test_rest_get(env, request_line, url, username, password, certification_path);

    /*POST方法测试*/
    method = "POST";
    path = "/ibm/director/rest/discover";
    request_body = "{\"IPAddress\":[\"192.168.1.1\"]}";

    request_line = test_request_line_init(env, method, path, http_version);
    url = test_url_init(env, protocol, host_address, port, path);
    test_rest_post(env, request_line, url, username, password, certification_path, request_body);

    /*PUT方法测试*/
    method = "PUT";
    path = "/ibm/director/rest/resources/Server/7303";
    request_body = "{\"Properties\":{\"DisplayName\":\"IBM 7872\",\"Description\":\"IBM machine\"}}";

    request_line = test_request_line_init(env, method, path, http_version);
    url = test_url_init(env, protocol, host_address, port, path);
    test_rest_put(env, request_line, url, username, password, certification_path, request_body);

    /*DELETE方法测试*/
    method = "DELETE";
    path = "/ibm/director/rest/resources/Server/7303";

    request_line = test_request_line_init(env, method, path, http_version);
    url = test_url_init(env, protocol, host_address, port, path);
    test_rest_delete(env, request_line, url, username, password, certification_path);

    axutil_env_free(env);
    return 0;
}
