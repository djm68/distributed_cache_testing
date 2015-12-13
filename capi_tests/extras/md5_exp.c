#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include </usr/include/openssl/evp.h>
#define MAX_SIZE 32


char *gen_rand_str(char *dst, int size);
void md5_gen(char *ptr, char *md5_ptr);

main(int argc, char *argv[])
{
  srand(time(0));

  int i, obj_len; 
  int count=5;
  char *big_obj = (char *)malloc(MAX_SIZE+1);
  char *obj_md5 = (char *)malloc(32);
  char **md5_arr; /* ptr to array of char arrays*/

  // allocate mem for pointers to the m5 hashes
  md5_arr = (char **)malloc(count*sizeof(char*));

  // allocate mem for actual array elements -- m5 hashes are 32 chars
  for (i=0; i < count; ++i)
      md5_arr[i] = (char *)malloc(32*sizeof(char));

  for (i=0; i < count; ++i)
    {
      obj_len = ( rand() % (MAX_SIZE) );
      if (obj_len < 5 )
        obj_len = 5;

      // gen a rand string length of obj_len
      gen_rand_str(big_obj, obj_len);
      printf("OBJ %s\n", big_obj);
 
      // md5 hash the string 
      md5_gen(big_obj, obj_md5);

      // save the md5 hash in md5_arr
      strncpy(md5_arr[i], obj_md5, 32);
    }

  for (i=0; i < count; ++i)
     printf("MD  %s\n",md5_arr[i]); 
  
  free(big_obj);
  free(obj_md5);
  for (i=0; i < count; ++i)
     free(md5_arr[i]); 
  free(md5_arr);
}


void md5_gen(char *obj_ptr, char *md5_ptr)
{
  EVP_MD_CTX mdctx;
  const EVP_MD *md;
  unsigned char md_value[EVP_MAX_MD_SIZE];
  int md_len, i;
  char *buf_ptr;

  OpenSSL_add_all_digests();
  md = EVP_get_digestbyname("md5");
  EVP_MD_CTX_init(&mdctx);
  EVP_DigestInit_ex(&mdctx, md, NULL);
  EVP_DigestUpdate(&mdctx, obj_ptr, strlen(obj_ptr));
  EVP_DigestFinal_ex(&mdctx, md_value, &md_len);
  EVP_MD_CTX_cleanup(&mdctx);

  buf_ptr = md5_ptr;
  for(i = 0; i < md_len; i++) buf_ptr+=sprintf(buf_ptr,"%02x",md_value[i]);
  printf("CNT 01234567890123456789012345678901\n");
  printf("MD5 %s\n", md5_ptr);
}


char *gen_rand_str(char *dst, int size)
{
  int i;
  static const char text[] = "abcdefghijklmnopqrstuvwxyz0123456789"
                             "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  for ( i = 0; i < size; ++i )
  {
    dst[i] = text[rand() % (sizeof text - 1)];
  }
  dst[i] = '\0';
  printf("DST %s\n", dst);
  return dst;
}
