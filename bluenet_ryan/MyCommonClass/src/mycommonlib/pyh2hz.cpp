// PYH2HZ.cpp : Implementation of CPYH2HZ
#include "stdafx.h"
#include "pyh2hz.h"
#include <ctype.h>

static const char * g_PY2TBL =\
        "cjwgnspgcgnepypbtyyzdxykygtpjnmjqmbsgzscyjsyyppgkbzgyp"\
        "ywjkgkljswkpjqhypwpdzlsgmrypywwcckznkyygttngjeykkzytcj"\
        "nmcylqlypyqfqrpzslwbtgkjfyxjwzltbncxjjjjzxdttsqzycdxxh"\
        "gckpphffsspybgmxlpbyllphlxspzmpjhsojnghdzqyklgjhsgqzhx"\
        "qgkezzwyscscjnyeyxadzpmdssmzjzqjyzcpjpwqjbdzbxgznzcpwh"\
        "kxhqkmwfbpbydtjzzkqhylygxfptyjyyzpszlfchmqshgmxxsxjppd"\
        "csbbqbefsjyhwwgzkpylqbgldlcctnmayddkssngycsgxlyzaypnpt"\
        "sdkdylhgymylcxpypjndqjwxqxfyyfjlejbzrxccqwqqsbzkymgplb"\
        "mjrqcflnymyqmsqtrbzjthztqfrxqhxmjjcjlxqgjmshzkbswyemyl"\
        "txfsydsglycjqxsjnqbsctyhbftdcyzdjwyghqfrxwckqkxebptlpx"\
        "jzsrmebwhjlbjslyysmdxlclqkxlhxjrzjmbqhxhwywsbhtrxxglhq"\
        "hfnmpykldyxzpplggpmtcfpajjzyljtyanjgbjplqgdzyqyaxbkyse"\
        "cjsznslyzhzxlzcghpxzhznytdsbcjkdlzayfmydlebbgqyzkggldn"\
        "dnyskjshdlyxbcghxypkdjmmzngmmclgwzszxzjfznmlzzthcsydbd"\
        "llscddnlkjykjsycjlkohqasdknhcsganhdaashtcplcpqybsdmpjl"\
        "pcjoqlcdhjjysprchnpjnlhlyyqyhwzptczgwwmzffjqqqqyxaclbh"\
        "kdjxdgmmydjxzllsygxgkjrywzwyclzmssjzldbydcfcxyhlxchyzj"\
        "qppqagmnyxpfrkssbjlyxysyglnscmhzwwmnzjjlxxhchsyppttxry"\
        "cyxbyhcsmxjsznpwgpxxtaybgajcxlypdccwzocwkccsbnhcpdyznf"\
        "cyytyckxkybsqkkytqqxfcwchcykelzqbsqyjqcclmthsywhmktlkj"\
        "lycxwheqqhtqhppqpqscfymmdmgbwhwlgsllysdlmlxpthmjhwljzy"\
        "hzjxhtxjlhxrswlwzjcbxmhzqxsdzpmgfcsglsxymjshxpjxwmyqks"\
        "myplrthbxftpmhyxlchlhlzylxgsssstclsldclrpbhzhxyyfhbpgd"\
        "mycnqqwlqhjjpywjzyejjdhpblqxtqkwhlchqxagtlxljxmslphtzk"\
        "zjecxjcjnmfbypsfywybjzgnysdzsqyrsljpclpwxsdwejbjcbcnay"\
        "twgmpapclyqpclzxsbnmsggfnzjjbzsfzyndxhplqkzczwalsbccjx"\
        "pyzgwkypsgxfzfcdkhjgxtlqfsgdslqwzkxtmhsbgzmjzrglyjbpml"\
        "msxlzjqqhzyjczydjwbmjklddpmjegxyhylxhlqyqhkycwcjmyyxna"\
        "tjhyccxzpcqlbzwwytwbqcmlpmyrjcccxfpznzzljplxxyztzlgdlt"\
        "cklyrzzgqtgjhhgjljaxfgfjzslcfdqzlclgjdjcsnclljpjqdcclc"\
        "jxmyzftsxgcgsbrzxjqqctzhgyqtjqqlzxjylylbcyamcstylpdjby"\
        "regklzyzhlyszqlznwczcllwjqjjjkdgjzolbbzppglghtgzxyghzm"\
        "ycnqcycyhbhgxkamtxyxnbskyzzgjzlqjdfcjxdygjqjjpmgwgjjqp"\
        "kqsbgbmmcjssclpqpdxcdyykypcjddyygywrhjrtgznyqldkljszzg"\
        "zqzjgdykshpzmtlcpwnjafyzdjcnmwescyglbtzcgmssllyxqsxsbs"\
        "jsbbsgghfjlwpmzjnlyywdqshzxtyywhmcyhywdbxbtlmsyyyfsxjc"\
        "pdxxlhjhfpsxzqhfzmqcztqcxzxrttdjhnnyzqqmtqdmmgpydxmjgd"\
        "hcdyzbffallztdltfxmxqzdngwqdbdczjdxbzgsqqddjcmbkzffxmk"\
        "dmdsyyszcmljdsynsprskmkmpcklgdbqtfzswtfgglyplljzhgjpgy"\
        "pzltcsmcnbtjbqfkthbyzgkpbbymtdssxtbnpdkleycjnycdykzddh"\
        "qhsdzsctarlltkzlgecllkjlqjaqnbdkkghpjtzqksecshalqfmmgj"\
        "nlyjbbtmlyzxdcjpldlpcqdhzycbzsczbzmsljflkrzjsnfrgjhxpd"\
        "hyjybzgdlqcsezgxlblgyxtwmabchecmwyjyzlljjyhlgpdjlslygk"\
        "dzpzxjyyzlwcxszfgwyydlyhcljscmbjhblyzlycblydpdqysxqzby"\
        "tdkyxjypcnrjmpdqgklcljbztbjddbblblczqrpyxjcglzcshltolj"\
        "nmdddlngkaqhqhjgykheznmshrppqqjchgmfprxhjgdychgklyrzql"\
        "cyqjnzsqtkqjymszswlcfqqqxyfggyptqwlmcrnfkkfsyylqbmqamm"\
        "myxctpshcptxxzzsmphpshmclmldqfyqxszyjdjjzzhqpdszglstjb"\
        "ckbxyqzjsgpsxqzqzrqtbdkyxzkhhgflbcsmdldgdzdblzyycxnncs"\
        "ybzbfglzzxswmsccmqnjqsbdqsjtxxmbltxzclzshzcxrqjgjylxzf"\
        "jphymzqqydfqjjlzznzjcdgzygctxmzysctlkphtxhtlbjxjlxscdq"\
        "xcbbtjfqzfsltjbtkqbxxjjljchczdbzjdczjdcprnpqcjpfczlclz"\
        "xzdmxmphjsgzgszzqjylwtjpfsyasmcjbtzyycwmytzsjjlqcqlwzm"\
        "albxyfbpnlsfhtgjwejjxxglljstgshjqlzfkcgnndszfdeqfhbsaq"\
        "tgylbxmmygszldydqmjjrgbjtkgdhgkblqkbdmbylxwcxyttybkmrt"\
        "jzxqjbhlmhmjjzmqasldcyxyqdlqcafywyxqhz";

/////////////////////////////////////////////////////////////////////////////
// CPYH2HZ

CPYH2HZ::CPYH2HZ()
{
}

///-------------------------------------------------------
/// 2002-10-26
/// 功能：
///
/// 入口参数：
///
/// 返回参数：
///		0				失败
///		其他			字母
char CPYH2HZ::HZTOPY(unsigned short wHZ)
{
	unsigned short wQuWei = (wHZ>>8) - 0xA0;				//	区位
	char cRetVal = 0;
	if( wQuWei >= 16 && wQuWei < 56 )			//	一级字库
		cRetVal = GetPyFormHZ1( wHZ ) ;
	else if( wHZ >= 0xA3B0 && wHZ <= 0xA3FF )
		return (char)( (wHZ&0xFF)-0x80 );		//	全角字母
	else
	{											//	二级字库
		unsigned short wOffset = (wQuWei-56) * 94;
		wOffset  += ( ((BYTE)wHZ) - 0xA1 );
		if( wOffset >= 3009 )
			return 0;
		cRetVal = toupper( g_PY2TBL[wOffset] );
	}
	return cRetVal;
}

//	获取一级字库的发音的第一个字母
//	返回参数
//		0				失败
//		其他			字母
char CPYH2HZ::GetPyFormHZ1(unsigned short wHZ)
{
	struct tagHZPYTBL
	{
		unsigned short	m_wHead;
		unsigned short	m_wTail;
		char	m_cPY;
	};
	static struct tagHZPYTBL aHzPyTbl1[]=
	{
		{0xB0A1,0xB0C4,'A'},
		{0xB0C5,0xB2C0,'B'},
		{0xB2C1,0xB4ED,'C'},
		{0xB4EE,0xB6E9,'D'},
		{0xB6EA,0xB7A1,'E'},
		{0xB7A2,0xB8C0,'F'},
		{0xB8C1,0xB9FD,'G'},
		{0xB9FE,0xBBF6,'H'},
		{0xBBF7,0xBFA5,'J'},
		{0xBFA6,0xC0AB,'K'},
		{0xC0AC,0xC2E7,'L'},
		{0xC2E8,0xC4C2,'M'},
		{0xC4C3,0xC5B5,'N'},
		{0xC5B6,0xC5BD,'O'},
		{0xC5BE,0xC6D9,'P'},
		{0xC6DA,0xC8BA,'Q'},
		{0xC8BB,0xC8F5,'R'},
		{0xC8F6,0xCBF9,'S'},
		{0xCBFA,0xCDD9,'T'},
		{0xCDDA,0xCEF3,'W'},
		{0xCEF4,0xD1B8,'X'},
		{0xD1B9,0xD4D0,'Y'},
		{0xD4D1,0xD7F9,'Z'}
	};
	int nCount = sizeof(aHzPyTbl1)/sizeof(struct tagHZPYTBL);
	for(int i=0; i<nCount; i++)
	{
		if( wHZ >= aHzPyTbl1[i].m_wHead && wHZ <= aHzPyTbl1[i].m_wTail )
			return aHzPyTbl1[i].m_cPY;
	}
	return 0;
}


