#include <stdio.h>
#include <stdlib.h>

#define TRUE 0
#define FALSE 1

typedef struct huf {
	long freq;
	int symbol;
	struct huf *left, *right;
} huf;


huf *Head[256];
huf *Huf_head;
int nHead;
long nFreq[256]; 

unsigned int nCode[256];
int nLen[256];

int string[100];
int bitloc = -1;
int i=0, c=0;

/*
Head            : 트리를 생성하기위해 노드들을 저장
Huf_head        : 허프만 트리의 루트를 저장
nHead           : 노드의 수를 저장
nFreq           : 아스키 각 문자에 대한 빈도수를 저장

nCode           : 각 ascii값에 해당하는 코드값들을 저장
nLen            : 각 코드 값에 대한 트리의 깊이.
*/


void Huffman_Comp(FILE *);

unsigned get_freq(FILE*);

void Make_Tree(void);
void Init_Node();
int Find_Min(void);

void Make_Code(void);
void Make_Code_2(huf *, unsigned , int );

void print_codeword();
void DecimalToBinary(unsigned int );

void put_bitseq(unsigned , FILE *, int );
unsigned bits(unsigned, int, int);


void Huffman_Decomp(FILE *, FILE *);

void Destruct_Tree(huf*);

void Restruct_Tree(void);
void Insert_Tree(int );

int Get_Bitseq(FILE *);



int main(void)
{
	FILE *src, *src2, *src3;
	int type;

	printf("Press the Number key < 1 or 2 > \n");
	printf("1 : Encoding\n2 : Decoding\n");

	scanf("%d", &type);
	
	/* Encoding */
	if (type == 1) 
	{
		if ((src = fopen("hw2_2_orig_doc.txt", "rb")) == NULL)
		{
			printf("\n Error. That File can not Found.");
			exit(1);
		}
		Huffman_Comp(src);

		printf("\nCongratulation! File compressed.\n");
		printf("Created 3 files.\n\n");
		printf("(1)hw2_2_encoded.bin\n(2)hw2_2_codeword.txt\n(3)hw2_2_bin_tree.txt\n\n");

		fclose(src);
	}

	/*Decoding*/
	else if (type == 2)
	{
		if ((src2 = fopen("hw2_2_encoded.bin", "rb")) == NULL)
		{
			printf("\n Error. That File can not Found.");
			exit(1);
		}
		
		if ((src3 = fopen("hw2_2_bin_tree.txt", "r")) == NULL)
		{
			printf("\n Error. That File can not Found.");
			exit(1);
		}
		
		Huffman_Decomp(src2, src3);
        printf("\nCongratulation! File decompressed & created.\n");
		printf("Created 1 file.\n\n(1)hw2_2_decoded.txt\n\n");
		fclose(src2);
		fclose(src3);
    }
	
	else
        printf("Please try Again.");
	
	return 0;
}


/* 비트의 부분을 뽑아내는 함수. 원하는 위치의 비트 하나를 넘겨줌 */
unsigned bits(unsigned x, int y, int j)
{
	return (x >> y) & ~(~0 << j);
}


/* 파일에 한 비트씩 출력하도록 캡슐화 한 함수 */
void put_bitseq(unsigned i, FILE *fp, int flag)
{
	static unsigned writebyte = 0;
	/* 8비트를 만들어서 저장 */
	if (bitloc < 0 || flag == FALSE)
	{
		putc(writebyte, fp);
		bitloc = 7;
		writebyte = 0;
	}
	writebyte |= i << (bitloc--);
}


/* 파일에 존재하는 문자들의 빈도수를 구해서 nFreq[]에 저장하는 함수 */
unsigned get_freq(FILE *fp)
{
	int i;
	unsigned enc_len = 0;

	for (i = 0; i < 256; i++) 
	nFreq[i] = 0L;
    
	rewind(fp);
	/* 화일을 읽어서 같은 문자에 하나씩증가 */
	while( !feof(fp) )
	{
		i=getc(fp);
		enc_len++;
		nFreq[i]++;
	}
	return enc_len - 1; /* A-Z 와 a-z 사이의 문자의 총 빈도 수를 리턴 */
}


/* 최소 빈도수를 찾는 함수 */
int Find_Min(void)
{
	int mini_index = 0;
	int i;
	/*빈도수가 가장 작은 노드를 찾음*/
	for (i = 1; i < nHead; i++)
		if (Head[i]->freq < Head[mini_index]->freq)
			mini_index = i;
	return mini_index;
}


/* 트리를 만들기 위해서 노드들을 초기화하는 함수 */
void Init_Node()
{
	int i=0;
	huf* temp;

	for (i = nHead = 0; i < 256; i++)
	{
		/* 빈도수가 0인것은 노드생성 안함 */
		if (nFreq[i] != 0)
		{
			temp = (huf*) malloc (sizeof(huf));

			temp->freq = nFreq[i];
			temp->symbol = i;
			temp->left = temp->right = NULL;

			/*Head에 현재 노드에 주소를 저장*/
			Head[nHead++] = temp;
        }
    }
}


/* nFreq[]로 허프만 트리를 구성하는 함수 */
void Make_Tree(void)
{
	int m;
	huf *temp, *h1, *h2;
	
	/*노드들의 초기화*/
	Init_Node();

	/* 허프만트리 생성 과정*/
	while (nHead > 1)
	{
		/*현재 노드중 가장 작은 빈도수를 가진 노드를 찾음*/
		m = Find_Min();
		h1 = Head[m];
		
		/*
		현재 노드에 맨마지막에 있는 노드의 주소값을 이동시킨다. 나중에는 head[0]만 남는다.
		*/
		Head[m] = Head[--nHead];

		/*
        위에서 현재 노드들 중에서 가장 작은 노드를 찾아서 그 자리에 놓았기 때문에 
		그 다음으로 가장 작은 노드를 찾는다.
        */
		m = Find_Min();
        h2 = Head[m];
    
		temp = (huf*)malloc(sizeof(huf));

		/*
		h1과 h2를 합쳐서 새로운 부모 노드를 생성한다. 
		부모노드는 ascii값이 없고, 자식으로 왼쪽, 오른쪽 노드에 h1, h2를 갖는다.
		*/
		temp->freq = h1->freq + h2->freq;
		temp->symbol = 0;
		temp->left = h1;
		temp->right = h2;
	
		/* 생성된 새로운 노드를 Head[nIndex]위치에 삽입한다. */
		Head[m] = temp;
	}
	
	/* 허프만 트리의 루트를 Head에 첫번째주소를 가리키게 함으로써 트리가 완성된다. */
	Huf_head = Head[0];
}


/* Huffman Tree를 제거하는 함수 */
void Destruct_Tree(huf *head)
{
	if (head != NULL)
	{
		Destruct_Tree(head->left);
		Destruct_Tree(head->right);
		free(head);
	}
}


/* make_code_2() 함수의 입구 함수 */
void Make_Code(void)
{
    int i;

    for (i = 0; i < 256; i++)
        nCode[i] = nLen[i] = 0;

    Make_Code_2(Huf_head, 0u, 0);
}


/* Huffman Tree에서 바이너리 코드를 얻어내기 위한 함수. */
void Make_Code_2(huf *h, unsigned code, int lenght)
{
	/*
	만약 잎사귀 노드가 아니면 탐색을 하면서 코드를 생성하고
	그렇지 않고 잎사귀 노드면 지금까지 탐색했던 결과를 저장한다.
	*/
	if (h->left != NULL || h->right != NULL)
	{
		/* 코드를 한칸 왼쪽으로 shift --> 0을 넣는 결과. 코드의 길이를 하나씩 증가. */
		code <<= 1;
		lenght++;

		/* 왼쪽 자식으로 재귀호출 */
		Make_Code_2(h->left, code, lenght);

		/* 왼쪽 노드 방문을 끝내고 오른쪽 노드로 갈때 코드에 1or연산을 통해 첨가 */
		code |= 1;
		
		/* 오른쪽 자식으로 재귀호출 */
		Make_Code_2(h->right, code, lenght);
		
		/* 부모로 돌아가기 위해서 다시 원상복귀 */
		code >>= 1;
		lenght--;
	}
	else
	{
		/* ascii 값에 재설정된 코드와 길이를 저장 */
		nCode[h->symbol] = code;
		nLen[h->symbol] = lenght;
    }
}


/* hw2_2_codword.txt 파일에 codeword와 빈도 정보를 출력하기 위한 함수. */
void print_codeword()
{
	FILE* fp2;
	float per;
	int k, a;
	
	fp2 = fopen("hw2_2_codeword.txt", "w");	

	for( k = 0 ; k < 256 ; k++ ) 
	{
        
        if( nLen[k] != 0 ) 
		{
			fprintf(fp2, "%c: (", k);
			DecimalToBinary(nCode[k]);
			if (c < nLen[k] )
			{
				a = c;
				while(nLen[k] - a > 0)
				{
					fprintf(fp2, "0");
					a++;
				}
			}
			for( i = 0 ; i < c ; i++)
				fprintf(fp2, "%d", string[i]);
			
			i=c=0;


			per = (float)nFreq[k]/(float)Huf_head->freq;
			fprintf(fp2, ", %.2f%c)", per*100, 37);
			fprintf(fp2, "\n");
		}
	}
	fclose(fp2);
}

/* 10진수를 2진 바이너리로 고치기 위한 함수 */
void DecimalToBinary(unsigned int bCode)
{
	if(bCode > 1) 
		DecimalToBinary(bCode / 2);
	
	string[i++] = bCode % 2;
	c++;
}



/* 위에 쓰인 함수들을 모두 써서 구현한 허프만 인코딩 함수 */
void Huffman_Comp(FILE *src)
{
	FILE *fp2, *fp3;

	int cur;
	int i;
	unsigned length, enc_length;

	int b;

	fseek(src, 0L, SEEK_END);
	length = ftell(src);

	rewind(src);

	fp2 = fopen("hw2_2_encoded.bin", "wb");

	fp3 = fopen("hw2_2_bin_tree.txt", "w");

	/* 압축 파일의 헤더 작성 */
    fwrite(&length, sizeof(unsigned int), 1, fp2);  /* 파일의 길이 출력 */

    enc_length = get_freq(src);
	
	/* 압축 파일의 헤더 작성 */
	fwrite(&enc_length, sizeof(unsigned int), 1, fp2); /* 인코딩 후 파일의 길이 출력 */

	/* 트리를 만들고 코드를 만들고 출력하고.. 등등.. 위의 주석 참조 */
	Make_Tree();
	Make_Code();
	print_codeword();

	/* nCode[]와 nLen[]의 출력 트리 정보. hw2_2_bin_tree.txt에 출력할 내용. */
	for( i = 0 ; i < 256 ; i++ ) 
	{
		putw(nCode[i], fp3);
		putw(nLen[i], fp3);
	}
	
	/* 허프만 트리 삭제 */
    Destruct_Tree(Huf_head);

    rewind(src);
    bitloc = 7; /*비트를 1byte를 만들기 위해서 필요 */
    
	/* 인코딩된 결과를 출력. hw2_2_encoded.bin에 출력할 내용 */
	while (1)
    {
        cur = getc(src);
        
        if (feof(src)) 
            break;
        
        for (b = nLen[cur] - 1; b >= 0; b--)
            put_bitseq(bits(nCode[cur], b, 1), fp2, TRUE);
    }
    put_bitseq(0, fp2, FALSE);
    
	fclose(fp2);
	fclose(fp3);
}



/* Insert_Tree()의 입구 함수 */
void Restruct_Tree(void)
{
	int i;
	Huf_head = NULL;
	for (i = 0; i < 256; i++)
		if (nLen[i] > 0) 
			Insert_Tree(i);
}


/* 앞서 저장한 hw2_2_bin_tree.txt에서 len[]와 code[]를 재구성하여 허프만 트리를 생성 */
void Insert_Tree(int data)
{
	int b = nLen[data] - 1;
	huf *t1, *t2;

	if (Huf_head == NULL)
	{
		Huf_head = (huf*)malloc(sizeof(huf));
		Huf_head->left = Huf_head->right = NULL;
	}

	t1 = t2 = Huf_head;
	while (b >= 0)
	{
		if (bits(nCode[data], b, 1) == 0)
		{
			t2 = t2->left;
			if (t2 == NULL)
			{
				t2 = (huf*)malloc(sizeof(huf));
				t2->left = t2->right = NULL;
				t1->left = t2;
			}
		}
		else
		{
			t2 = t2->right;
			if (t2 == NULL)
			{
				t2 = (huf*)malloc(sizeof(huf));
				t2->left = t2->right = NULL;
				t1->right = t2;
			}
		}
		t1 = t2;
		b--;
	}
	t2->symbol = data;
}


/* 파일에서 한 비트씩 읽는 것처럼 캡슐화 한 함수 */
int Get_Bitseq(FILE *fp)
{
	static int cur = 0;

	if (bitloc < 0)
	{
		cur = getc(fp);
		bitloc = 7;
	}
	return bits(cur, bitloc--, 1);
}


/* 위의 함수들을 모두 써서 작성한 허프만 디코딩 함수. */
void Huffman_Decomp(FILE *src2, FILE *src3)
{
	FILE *fp;
	long length, enc_length;
	long n;
	huf *h;
	int i = 0;

	rewind(src2);
	
	fp = fopen("hw2_2_decoded.txt", "wb");

	fread(&length,sizeof(unsigned int),1,src2);
	fread(&enc_length,sizeof(unsigned int),1,src2);

	/* nCode[]와 nLen[]을 hw2_2_bin_tree.txt에서 읽어들임 */

	for( i = 0 ; i < 256 ; i++ ) 
	{
		nCode[i] = getw(src3);
		nLen[i] = getw(src3);
	}	
    
	Restruct_Tree(); /* 헤더를 읽어서 허프만 트리의 재구성 */

	n = 0;
	bitloc = -1;

	while (n < enc_length)
	{
		h = Huf_head;
		while (h->left && h->right)
		{
			if (Get_Bitseq(src2) == 1)
				h = h->right;
			else
				h = h->left;
		}
		putc(h->symbol, fp);
		n++;
	}
    
	Destruct_Tree(Huf_head);
    
	fclose(fp);
}


