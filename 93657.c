	UriSuite() {
		TEST_ADD(UriSuite::testDistinction)
		TEST_ADD(UriSuite::testIpFour)
		TEST_ADD(UriSuite::testIpSixPass)
		TEST_ADD(UriSuite::testIpSixFail)
		TEST_ADD(UriSuite::testUri)
		TEST_ADD(UriSuite::testUriUserInfoHostPort1)
		TEST_ADD(UriSuite::testUriUserInfoHostPort2)
		TEST_ADD(UriSuite::testUriUserInfoHostPort22_Bug1948038)
		TEST_ADD(UriSuite::testUriUserInfoHostPort23_Bug3510198_1)
		TEST_ADD(UriSuite::testUriUserInfoHostPort23_Bug3510198_2)
		TEST_ADD(UriSuite::testUriUserInfoHostPort23_Bug3510198_3)
		TEST_ADD(UriSuite::testUriUserInfoHostPort23_Bug3510198_4)
		TEST_ADD(UriSuite::testUriUserInfoHostPort23_Bug3510198_related_1)
		TEST_ADD(UriSuite::testUriUserInfoHostPort23_Bug3510198_related_12)
		TEST_ADD(UriSuite::testUriUserInfoHostPort23_Bug3510198_related_2)
		TEST_ADD(UriSuite::testUriUserInfoHostPort3)
		TEST_ADD(UriSuite::testUriUserInfoHostPort4)
		TEST_ADD(UriSuite::testUriUserInfoHostPort5)
		TEST_ADD(UriSuite::testUriUserInfoHostPort6)
		TEST_ADD(UriSuite::testUriHostRegname)
		TEST_ADD(UriSuite::testUriHostIpFour1)
		TEST_ADD(UriSuite::testUriHostIpFour2)
		TEST_ADD(UriSuite::testUriHostIpSix1)
		TEST_ADD(UriSuite::testUriHostIpSix2)
		TEST_ADD(UriSuite::testUriHostIpFuture)
		TEST_ADD(UriSuite::testUriHostEmpty)
		TEST_ADD(UriSuite::testUriComponents)
		TEST_ADD(UriSuite::testUriComponents_Bug20070701)
		TEST_ADD(UriSuite::testEscaping)
		TEST_ADD(UriSuite::testUnescaping)
		TEST_ADD(UriSuite::testTrailingSlash)
		TEST_ADD(UriSuite::testAddBase)
		TEST_ADD(UriSuite::testToString)
		TEST_ADD(UriSuite::testToString_Bug1950126)
		TEST_ADD(UriSuite::testToStringCharsRequired)
		TEST_ADD(UriSuite::testToStringCharsRequired)
		TEST_ADD(UriSuite::testNormalizeSyntaxMaskRequired)
		TEST_ADD(UriSuite::testNormalizeSyntax)
		TEST_ADD(UriSuite::testNormalizeSyntaxComponents)
		TEST_ADD(UriSuite::testNormalizeCrash_Bug20080224)
		TEST_ADD(UriSuite::testFilenameUriConversion)
		TEST_ADD(UriSuite::testCrash_FreeUriMembers_Bug20080116)
		TEST_ADD(UriSuite::testCrash_Report2418192)
		TEST_ADD(UriSuite::testPervertedQueryString);
		TEST_ADD(UriSuite::testQueryStringEndingInEqualSign_NonBug32);
		TEST_ADD(UriSuite::testCrash_MakeOwner_Bug20080207)
 		TEST_ADD(UriSuite::testQueryList)
 		TEST_ADD(UriSuite::testQueryListPair)
 		TEST_ADD(UriSuite::testQueryDissection_Bug3590761)
		TEST_ADD(UriSuite::testQueryCompositionMathWrite_GoogleAutofuzz113244572)
 		TEST_ADD(UriSuite::testFreeCrash_Bug20080827)
 		TEST_ADD(UriSuite::testParseInvalid_Bug16)
 		TEST_ADD(UriSuite::testRangeComparison)
		TEST_ADD(UriSuite::testRangeComparison_RemoveBaseUri_Issue19)
		TEST_ADD(UriSuite::testEquals)
		TEST_ADD(UriSuite::testHostTextTermination_Issue15)
	}
