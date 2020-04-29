/************************************************************************************************
 *
 *	My RUDP OpenSSL server side helper
 *
 *	Chen Yongjian @ Zhoi
 *	2015.4.18 @ Xi'an
 *
 ************************************************************************************************/

#ifndef __MYRUDP_OPENSSL_SERVER_HELPER_H_20150418__
#define __MYRUDP_OPENSSL_SERVER_HELPER_H_20150418__

#include <vector>
#include <MySyncObj.h>

class CMyRUDPCryption;

class CMyRUDPOpenSSLServerHelper
{
public:
	CMyRUDPOpenSSLServerHelper();
	virtual ~CMyRUDPOpenSSLServerHelper();

	//--------------------------------------------------------------
	/** CYJ 2015-04-18
	 *
	 *	Create server side public key object
	 *
	 * @return	0					succ
	 *			other				error code
	 */
	int Initialize();

	//--------------------------------------------------------------
	/** CYJ 2015-04-18
	 *
	 *	Free resource
	 */
	void Invalidate();

	//--------------------------------------------------------------
	/** CYJ 2015-04-18
	 *
	 *	On date chaned, should renew server side public key
	 *
	 * @param [in]	tNow			current time
	 */
	void OnDateChanged( time_t tNow );

	//--------------------------------------------------------------
	/** CYJ 2015-04-18
	 *
	 *	Get encrypted servier side public key
	 *
	 * @param [in]	byType			ECC type
	 * @param [in]	pbyXorData		Xor data to encrypted the server side public key
	 * @param [in]	nSessionID		session ID, used as AES IV
	 *
	 * @return		encrypted server side public key
	 */
	std::vector<uint8_t> GetEncryptedPubKey( uint8_t byType, const uint8_t pbyXorData[32], int nSessionID );

	//--------------------------------------------------------------
	/** CYJ 2015-04-18
	 *
	 *	Get AES Key
	 *
	 * @param [in]	pbyPeerPubKey		peer public key, first by is type
	 * @param [in]	nLen				peer public key length
	 * @param [in]	pbyXorData			Xor data
	 * @param [out]	byECCType			output ECC type
	 *
	 * @return	AES Key
	 *
	 * @note
	 *		pbyPeerPubKey	is encrypted AES-128, using XorData
	 */
	std::vector<uint8_t> GetAESKey( const uint8_t * pbyPeerPubKey, int nLen, const uint8_t pbyXorData[32], uint8_t & byECCType );

	enum{
		MAX_OPENSSL_OBJECT_NUMBER = 1,
	};

protected:
	CMyRUDPCryption * m_apOpenSSLObj[ MAX_OPENSSL_OBJECT_NUMBER ];

	CReadWriteLock	m_SyncObj;
};

#endif // __MYRUDP_OPENSSL_SERVER_HELPER_H_20150418__
