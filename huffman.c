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
Head            : Ʈ���� �����ϱ����� ������ ����
Huf_head        : ������ Ʈ���� ��Ʈ�� ����
nHead           : ����� ���� ����
nFreq           : �ƽ�Ű �� ���ڿ� ���� �󵵼��� ����

nCode           : �� ascii���� �ش��ϴ� �ڵ尪���� ����
nLen            : �� �ڵ� ���� ���� Ʈ���� ����.
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


/* ��Ʈ�� �κ��� �̾Ƴ��� �Լ�. ���ϴ� ��ġ�� ��Ʈ �ϳ��� �Ѱ��� */
unsigned bits(unsigned x, int y, int j)
{
	return (x >> y) & ~(~0 << j);
}


/* ���Ͽ� �� ��Ʈ�� ����ϵ��� ĸ��ȭ �� �Լ� */
void put_bitseq(unsigned i, FILE *fp, int flag)
{
	static unsigned writebyte = 0;
	/* 8��Ʈ�� ���� ���� */
	if (bitloc < 0 || flag == FALSE)
	{
		putc(writebyte, fp);
		bitloc = 7;
		writebyte = 0;
	}
	writebyte |= i << (bitloc--);
}


/* ���Ͽ� �����ϴ� ���ڵ��� �󵵼��� ���ؼ� nFreq[]�� �����ϴ� �Լ� */
unsigned get_freq(FILE *fp)
{
	int i;
	unsigned enc_len = 0;

	for (i = 0; i < 256; i++) 
	nFreq[i] = 0L;
    
	rewind(fp);
	/* ȭ���� �о ���� ���ڿ� �ϳ������� */
	while( !feof(fp) )
	{
		i=getc(fp);
		enc_len++;
		nFreq[i]++;
	}
	return enc_len - 1; /* A-Z �� a-z ������ ������ �� �� ���� ���� */
}


/* �ּ� �󵵼��� ã�� �Լ� */
int Find_Min(void)
{
	int mini_index = 0;
	int i;
	/*�󵵼��� ���� ���� ��带 ã��*/
	for (i = 1; i < nHead; i++)
		if (Head[i]->freq < Head[mini_index]->freq)
			mini_index = i;
	return mini_index;
}


/* Ʈ���� ����� ���ؼ� ������ �ʱ�ȭ�ϴ� �Լ� */
void Init_Node()
{
	int i=0;
	huf* temp;

	for (i = nHead = 0; i < 256; i++)
	{
		/* �󵵼��� 0�ΰ��� ������ ���� */
		if (nFreq[i] != 0)
		{
			temp = (huf*) malloc (sizeof(huf));

			temp->freq = nFreq[i];
			temp->symbol = i;
			temp->left = temp->right = NULL;

			/*Head�� ���� ��忡 �ּҸ� ����*/
			Head[nHead++] = temp;
        }
    }
}


/* nFreq[]�� ������ Ʈ���� �����ϴ� �Լ� */
void Make_Tree(void)
{
	int m;
	huf *temp, *h1, *h2;
	
	/*������ �ʱ�ȭ*/
	Init_Node();

	/* ������Ʈ�� ���� ����*/
	while (nHead > 1)
	{
		/*���� ����� ���� ���� �󵵼��� ���� ��带 ã��*/
		m = Find_Min();
		h1 = Head[m];
		
		/*
		���� ��忡 �Ǹ������� �ִ� ����� �ּҰ��� �̵���Ų��. ���߿��� head[0]�� ���´�.
		*/
		Head[m] = Head[--nHead];

		/*
        ������ ���� ���� �߿��� ���� ���� ��带 ã�Ƽ� �� �ڸ��� ���ұ� ������ 
		�� �������� ���� ���� ��带 ã�´�.
        */
		m = Find_Min();
        h2 = Head[m];
    
		temp = (huf*)malloc(sizeof(huf));

		/*
		h1�� h2�� ���ļ� ���ο� �θ� ��带 �����Ѵ�. 
		�θ���� ascii���� ����, �ڽ����� ����, ������ ��忡 h1, h2�� ���´�.
		*/
		temp->freq = h1->freq + h2->freq;
		temp->symbol = 0;
		temp->left = h1;
		temp->right = h2;
	
		/* ������ ���ο� ��带 Head[nIndex]��ġ�� �����Ѵ�. */
		Head[m] = temp;
	}
	
	/* ������ Ʈ���� ��Ʈ�� Head�� ù��°�ּҸ� ����Ű�� �����ν� Ʈ���� �ϼ��ȴ�. */
	Huf_head = Head[0];
}


/* Huffman Tree�� �����ϴ� �Լ� */
void Destruct_Tree(huf *head)
{
	if (head != NULL)
	{
		Destruct_Tree(head->left);
		Destruct_Tree(head->right);
		free(head);
	}
}


/* make_code_2() �Լ��� �Ա� �Լ� */
void Make_Code(void)
{
    int i;

    for (i = 0; i < 256; i++)
        nCode[i] = nLen[i] = 0;

    Make_Code_2(Huf_head, 0u, 0);
}


/* Huffman Tree���� ���̳ʸ� �ڵ带 ���� ���� �Լ�. */
void Make_Code_2(huf *h, unsigned code, int lenght)
{
	/*
	���� �ٻ�� ��尡 �ƴϸ� Ž���� �ϸ鼭 �ڵ带 �����ϰ�
	�׷��� �ʰ� �ٻ�� ���� ���ݱ��� Ž���ߴ� ����� �����Ѵ�.
	*/
	if (h->left != NULL || h->right != NULL)
	{
		/* �ڵ带 ��ĭ �������� shift --> 0�� �ִ� ���. �ڵ��� ���̸� �ϳ��� ����. */
		code <<= 1;
		lenght++;

		/* ���� �ڽ����� ���ȣ�� */
		Make_Code_2(h->left, code, lenght);

		/* ���� ��� �湮�� ������ ������ ���� ���� �ڵ忡 1or������ ���� ÷�� */
		code |= 1;
		
		/* ������ �ڽ����� ���ȣ�� */
		Make_Code_2(h->right, code, lenght);
		
		/* �θ�� ���ư��� ���ؼ� �ٽ� ���󺹱� */
		code >>= 1;
		lenght--;
	}
	else
	{
		/* ascii ���� �缳���� �ڵ�� ���̸� ���� */
		nCode[h->symbol] = code;
		nLen[h->symbol] = lenght;
    }
}


/* hw2_2_codword.txt ���Ͽ� codeword�� �� ������ ����ϱ� ���� �Լ�. */
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

/* 10������ 2�� ���̳ʸ��� ��ġ�� ���� �Լ� */
void DecimalToBinary(unsigned int bCode)
{
	if(bCode > 1) 
		DecimalToBinary(bCode / 2);
	
	string[i++] = bCode % 2;
	c++;
}



/* ���� ���� �Լ����� ��� �Ἥ ������ ������ ���ڵ� �Լ� */
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

	/* ���� ������ ��� �ۼ� */
    fwrite(&length, sizeof(unsigned int), 1, fp2);  /* ������ ���� ��� */

    enc_length = get_freq(src);
	
	/* ���� ������ ��� �ۼ� */
	fwrite(&enc_length, sizeof(unsigned int), 1, fp2); /* ���ڵ� �� ������ ���� ��� */

	/* Ʈ���� ����� �ڵ带 ����� ����ϰ�.. ���.. ���� �ּ� ���� */
	Make_Tree();
	Make_Code();
	print_codeword();

	/* nCode[]�� nLen[]�� ��� Ʈ�� ����. hw2_2_bin_tree.txt�� ����� ����. */
	for( i = 0 ; i < 256 ; i++ ) 
	{
		putw(nCode[i], fp3);
		putw(nLen[i], fp3);
	}
	
	/* ������ Ʈ�� ���� */
    Destruct_Tree(Huf_head);

    rewind(src);
    bitloc = 7; /*��Ʈ�� 1byte�� ����� ���ؼ� �ʿ� */
    
	/* ���ڵ��� ����� ���. hw2_2_encoded.bin�� ����� ���� */
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



/* Insert_Tree()�� �Ա� �Լ� */
void Restruct_Tree(void)
{
	int i;
	Huf_head = NULL;
	for (i = 0; i < 256; i++)
		if (nLen[i] > 0) 
			Insert_Tree(i);
}


/* �ռ� ������ hw2_2_bin_tree.txt���� len[]�� code[]�� �籸���Ͽ� ������ Ʈ���� ���� */
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


/* ���Ͽ��� �� ��Ʈ�� �д� ��ó�� ĸ��ȭ �� �Լ� */
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


/* ���� �Լ����� ��� �Ἥ �ۼ��� ������ ���ڵ� �Լ�. */
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

	/* nCode[]�� nLen[]�� hw2_2_bin_tree.txt���� �о���� */

	for( i = 0 ; i < 256 ; i++ ) 
	{
		nCode[i] = getw(src3);
		nLen[i] = getw(src3);
	}	
    
	Restruct_Tree(); /* ����� �о ������ Ʈ���� �籸�� */

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


