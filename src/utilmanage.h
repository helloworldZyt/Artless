#ifndef __BDM_MANNER_H_
#define __BDM_MANNER_H_

#define MAN_ERR_CONVENTIONS_CEIL		999				// platform system errors

#define MANERRORS	{ \
	{0, "success"}, \
	{1001, "param error!"}, \
};

typedef enum _manage_api_{
	MANN_UTIL_API_UNKWON=0,
	MANN_UTIL_API_GET_IPS,
	MANN_UTIL_API_IP_CONNECTIVITY,
	MANN_UTIL_API_IP_PING,
}UtilApiIds;

/**
 *@desc			:
 *@eid			: entry id
 *@iv			: input param
 *@ov			: output param
 */
int utilman_entry(UtilApiIds eid, void* iv, void* ov);

#endif // __BDM_MANNER_H_