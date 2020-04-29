/***************************************************************************************
 *
 *	RelaxTalk Open SSL ( ECDH )
 *
 *	Chen Yongjian @ Zhoi
 *	2015.4.13 @ Xi'an
 *
 **************************************************************************************/

#ifndef __MYRUDP_OPENSSL_H_20150413__
#define __MYRUDP_OPENSSL_H_20150413__

#include <vector>
#include <stdint.h>

typedef struct ec_key_st 		EC_KEY;
typedef struct ec_point_st 		EC_POINT;
typedef struct ec_group_st		EC_GROUP;

/////////////////////////////////////////////////////////////////////////////
class CMyRUDPCryption
{
public:
	CMyRUDPCryption();
	virtual ~CMyRUDPCryption();

	//--------------------------------------------------------------
	/** CYJ 2015-04-13
	 *
	 *	Initialize ECDH key objct
	 *
	 * @param [in]	byType			type
	 *
	 * @return		0				succ
	 *				other			error code
	 */
	int Initialize( uint8_t byType );

	//--------------------------------------------------------------
	/** CYJ 2015-04-13
	 *
	 *	Invalidate and free resource
	 */
	void Invalidate();

	//--------------------------------------------------------------
	/** CYJ 2015-04-18
	 *
	 *	Is valid
	 */
	bool IsValid(){ return m_pKey != NULL; }

	//--------------------------------------------------------------
	/** CYJ 2015-04-13
	 *
	 *	Get Public key as Byte array
	 *
	 * @return	PubKey array
	 */
	const std::vector<uint8_t> & GetPubKey() { return m_aPubKey; }

	//--------------------------------------------------------------
	/** CYJ 2015-04-13
	 *
	 *	Decrypt AES Key
	 *
	 * @param [in]	pPeerPubKey		Peer Public Key
	 * @param [in]	nLen			Peer Public Key Length
	 *
	 * @return	AES Key
	 *
	 * @note
	 *		First byte is ECDH key type, see also MYRUDP_ECDH_TYPE_xxx
	 */
	std::vector<uint8_t> DecryptAESKey( const uint8_t * pPeerPubKey, int nLen );

	enum
	{
		MYRUDP_ECDH_TYPE_INVALID = 0,
		MYRUDP_ECDH_TYPE_384,
		MYRUDP_ECDH_TYPE_521,
	};
protected:
	int BuildPublicKey();

protected:
	EC_KEY 				* 	m_pKey;
	const EC_POINT 		* 	m_pPoint;
	const EC_GROUP		* 	m_pGroup;
	uint8_t			  		m_byKeyType;
	std::vector<uint8_t>	m_aPubKey;
};

/////////////////////////////////////////////////////////////////////
//--------------------------------------------------------------
/** CYJ 2015-04-18
 *
 *	Initialize OpenSSL multi-thread support
 *
 * @return	0				succ
 *			other			error code
 */
int MyRUDP_OpenSSL_Multithread_Init();

//--------------------------------------------------------------
/** CYJ 2015-04-18
 *
 *	Cleanup OpenSSL multi-thread support
 */
void MyRUDP_OpenSSL_Multithread_Cleanup();

#endif // __MYRUDP_OPENSSL_H_20150413__


