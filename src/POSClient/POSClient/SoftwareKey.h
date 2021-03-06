enum ProductType{PRODUCT_POS=1,PRODUCT_KDS,PRODUCT_RETAIL};
class CSoftwareKey
{
public:
	CSoftwareKey(void);
	~CSoftwareKey(void);
	int m_nKeyType;//1-	安捷代理版；2-	聚客版；3-	英文版

/************************************************************************
* 函数介绍：生成userid
* 输入参数：
* 输出参数：
* 返回值  ：
************************************************************************/
	CString GenUserID();
/************************************************************************
* 函数介绍：验证产品密钥是否正确
* 输入参数：strKeyFile	- 产品密钥文件路径
* 输出参数：strExpire	- 如果是正确的密钥，传出到期时间
* 返回值  ：0  - 成功
			-1 - 不合法的密钥
			-2 - 密钥过期
			-3 - userid不匹配
			-4 - 未找到注册文件
************************************************************************/
    int VerifySoftwareKey(LPCTSTR strKeyFile,ProductType type,CString& strExpire);


/************************************************************************
* 函数介绍：获取注册的手机数目
* 输入参数：strKeyFile	- 产品密钥文件路径
* 输出参数：padNums	- 平板数
			dcbNums - 手机数
			大于 0  - 注册的手机数量
			-1 - 不合法的密钥
			-2 - 密钥过期
			-3 - userid不匹配
			-4 - 未找到注册文件
*返回值  ： 到期时间
************************************************************************/
    CString GetRegedDevices(LPCTSTR strKeyFile,int& padNums,int& dcbNums);


/************************************************************************
* 函数介绍：获取注册的自助点菜数目
* 输入参数：strKeyFile	- 产品密钥文件路径
* 输出参数：padNums	- 自助点菜数
* 返回值  ：大于 0  - 注册的手机数量
			-1 - 不合法的密钥
			-2 - 密钥过期
			-3 - userid不匹配
			-4 - 未找到注册文件
*返回值  ： 到期时间
************************************************************************/
    CString GetRegediPad(LPCTSTR strKeyFile,int& padNums);
};
