#pragma once
// thank you! http://nlp.kookmin.ac.kr/data/han-code.html
namespace charset
{
	extern unsigned short _get_compo(int i, unsigned char cp);
	extern unsigned short uk2j_sub(unsigned short uc, unsigned char _from_ks);
	extern unsigned short j2uk_sub(unsigned short kssm_code, unsigned char _to_ks);
	extern unsigned short _unicode_to_KSSM(unsigned short uc);
	extern unsigned short _KSSM_to_unicode(unsigned short kc);
	extern int get_oem_code_page(unsigned short code);
	extern unsigned short _KS_to_unicode(unsigned short ksc);
	extern unsigned short _unicode_to_KS(unsigned short code);
}