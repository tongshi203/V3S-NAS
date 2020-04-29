#ifndef __SHELL_SORT_TEMPLATE_H_20081015__
#define __SHELL_SORT_TEMPLATE_H_20081015__
//≈≈–Ú 
//≈≈–Ú
template <class T> void ShellSort(T Items[], int nCount, BOOL bAscend )
{
	T tmp;
	int i,k,j;
	k = nCount / 2;
	while( k>0 )
	{	
		for( j=k; j<=nCount-1; j++ )
		{
			tmp = Items[j];
			i = j - k;
			if ( bAscend ) 
			{
				while( (i>=0) && (Items[i] > tmp) ) 
				{
					Items[ i+k ] = Items[ i ];
					i = i-k;
				}
			}
			else
			{
				while( (i>=0) && (Items[i] < tmp) )
				{
					Items[i+k] = Items[i];
					i = i - k;
				}
			}
			Items[ k+i ] = tmp;
		}
		k = k/2;
	}
}

#endif // __SHELL_SORT_TEMPLATE_H_20081015__
