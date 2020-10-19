#include "stdafx.h"
#include "GlUtils.h"

/**
 * @brief һά����ӳ�亯������[a,b]�еĵ�x��ӳ�䵽[a1,b1]�еĵ�x1
 *
 **/
float LinearMap(float x, float a, float b, float a1, float b1)
{
	auto srcDelt(b - a);
	if (FuzzyIsZero(srcDelt))
	{
		// ԭʼ��Χʱ0,
		//LOG_WARN("ԭʼ��Χ����Ϊ0(%f,%f)", a, b);
		return a1;
	}

	return (b1 - a1) / srcDelt * (x - a) + a1;
}