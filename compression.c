#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "huffman.h"
#define INTERNAL 1
#define LEAF 0

typedef struct node
{
	char x;
	int freq;
	char *code;
	int type;
	struct node *next;
	struct node *left;
	struct node *right;
} node;

node *HEAD, *ROOT;
int total_code_length = 0;
int encoded_message_length = 0;

void printll();
void makeTree();
void genCode(node *p, char *code);
void insert(node *p, node *m);
void addSymbol(char c);
void writeHeader(FILE *f);
void writeBit(int b, FILE *f);
void writeCode(char ch, FILE *f);
void printHuffmanTree(node *p, int level);
void writecode(char ch, FILE *f);
char *getCode(char ch);

node *newNode(char c)
{
	node *q;
	q = (node *)malloc(sizeof(node));
	q->x = c;
	q->type = LEAF;
	q->freq = 1;
	q->next = NULL;
	q->left = NULL;
	q->right = NULL;
	return q;
}

void printll()
{
	node *p;
	p = HEAD;

	while (p != NULL)
	{
		printf("[%c|%d]=>", p->x, p->freq);
		p = p->next;
	}
}

int main(int argc, char *argv[])
{
	FILE *fp, *fp2;
	char ch;
	int t;
	HEAD = NULL;
	ROOT = NULL;
	if (argc <= 2)
	{
		printf("Usage:\n %s <input-file-to-zip> <zipped-output-file>\n\n", argv[0]);
		if (argc == 2)
		{
			argv[2] = (char *)malloc(sizeof(char) * (strlen(argv[1]) + strlen(ext) + 1));
			strcpy(argv[2], argv[1]);
			strcat(argv[2], ext);
			argc++;
		}
		else
			return 0;
	}
	fp = fopen(argv[1], "rb");
	if (fp == NULL)
	{
		printf("[!]Input file cannot be opened.\n");
		return -1;
	}

	printf("\n[Pass1]");
	printf("\nReading input file %s", argv[1]);
	while (fread(&ch, sizeof(char), 1, fp) != 0)
		addSymbol(ch);
	fclose(fp);

	printf("\nConstructing Huffman-Tree..");
	makeTree();
	printf("\nAssigning Codewords.\n");
	genCode(ROOT, "\0");

	printf("\n[Pass2]");
	fp = fopen(argv[1], "r");
	if (fp == NULL)
	{
		printf("\n[!]Input file cannot be opened.\n");
		return -1;
	}
	fp2 = fopen(argv[2], "wb");
	if (fp2 == NULL)
	{
		printf("\n[!]Output file cannot be opened.\n");
		return -2;
	}

	printf("\nReading input file %s", argv[1]);
	printf("\nWriting file %s", argv[2]);
	printf("\nWriting File Header.");
	writeHeader(fp2);
	printf("\nWriting compressed content.");
	while (fread(&ch, sizeof(char), 1, fp) != 0)
	{

		writeCode(ch, fp2);
	}

	printHuffmanTree(ROOT, 0);
	writecode(ch, fp2);
	printf("\nAverage Code Length: %.2f", (float)total_code_length / encoded_message_length);
	printf("\nLength of Huffman Encoded Message: %d", encoded_message_length);
	fclose(fp);
	fclose(fp2);

	printf("\nDone..\n");
	return 0;
}

void writeHeader(FILE *f)
{
	symCode record;
	node *p;
	int temp = 0, i = 0;
	p = HEAD;
	while (p != NULL)
	{
		temp += (strlen(p->code)) * (p->freq);
		if (strlen(p->code) > MAX)
			printf("\n[!] Codewords are longer than usual.");
		temp %= 8;
		i++;
		p = p->next;
	}

	if (i == 256)
		N = 0;
	else
		N = i;

	fwrite(&N, sizeof(unsigned char), 1, f);
	printf("\nN=%u", i);

	p = HEAD;
	while (p != NULL)
	{
		record.x = p->x;
		strcpy(record.code, p->code);
		fwrite(&record, sizeof(symCode), 1, f);

		p = p->next;
	}

	padding = 8 - (char)temp;
	fwrite(&padding, sizeof(char), 1, f);
	printf("\nPadding=%d", padding);

	for (i = 0; i < padding; i++)
		writeBit(0, f);
}

void writeCode(char ch, FILE *f)
{
	char *code;
	code = getCode(ch);

	while (*code != '\0')
	{
		if (*code == '1')
			writeBit(1, f);
		else
			writeBit(0, f);
		code++;
	}
	return;
}

void writeBit(int b, FILE *f)
{
	static char byte;
	static int cnt;
	char temp;

	if (b == 1)
	{
		temp = 1;
		temp = temp << (7 - cnt);
		byte = byte | temp;
	}
	cnt++;

	if (cnt == 8)
	{

		fwrite(&byte, sizeof(char), 1, f);
		cnt = 0;
		byte = 0;
		return;
	}
	return;
}

char *getCode(char ch)
{
	node *p = HEAD;
	while (p != NULL)
	{
		if (p->x == ch)
			return p->code;
		p = p->next;
	}
	return NULL;
}

void insert(node *p, node *m)
{

	if (m->next == NULL)
	{
		m->next = p;
		return;
	}
	while (m->next->freq < p->freq)
	{
		m = m->next;
		if (m->next == NULL)
		{
			m->next = p;
			return;
		}
	}
	p->next = m->next;
	m->next = p;
}

void addSymbol(char c)
{
	node *p, *q, *m;
	int t;

	if (HEAD == NULL)
	{
		HEAD = newNode(c);
		return;
	}
	p = HEAD;
	q = NULL;
	if (p->x == c)
	{
		p->freq += 1;
		if (p->next == NULL)
			return;
		if (p->freq > p->next->freq)
		{
			HEAD = p->next;
			p->next = NULL;
			insert(p, HEAD);
		}
		return;
	}

	while (p->next != NULL && p->x != c)
	{
		q = p;
		p = p->next;
	}

	if (p->x == c)
	{
		p->freq += 1;
		if (p->next == NULL)
			return;
		if (p->freq > p->next->freq)
		{
			m = p->next;
			q->next = p->next;
			p->next = NULL;
			insert(p, HEAD);
		}
	}
	else
	{
		q = newNode(c);
		q->next = HEAD;
		HEAD = q;
	}
}

void makeTree()
{
	node *p, *q;
	p = HEAD;
	while (p != NULL)
	{
		q = newNode('@');
		q->type = INTERNAL;
		q->left = p;
		q->freq = p->freq;
		if (p->next != NULL)
		{
			p = p->next;
			q->right = p;
			q->freq += p->freq;
		}
		p = p->next;
		if (p == NULL)
			break;

		if (q->freq <= p->freq)
		{
			q->next = p;
			p = q;
		}
		else
			insert(q, p);
	}
	ROOT = q;
}

void genCode(node *p, char *code)
{
	char *lcode, *rcode;
	static node *s;
	static int flag;
	if (p != NULL)
	{

		if (p->type == LEAF)
		{
			if (flag == 0)
			{
				flag = 1;
				HEAD = p;
			}
			else
			{
				s->next = p;
			}
			p->next = NULL;
			s = p;
		}

		p->code = code;

		lcode = (char *)malloc(strlen(code) + 2);
		rcode = (char *)malloc(strlen(code) + 2);
		sprintf(lcode, "%s0", code);
		sprintf(rcode, "%s1", code);

		genCode(p->left, lcode);
		genCode(p->right, rcode);
	}
}
void printHuffmanTree(node *p, int level)
{
	if (p == NULL)
	{
		return;
	}
	printHuffmanTree(p->right, level + 1);
	for (int i = 0; i < level; i++)
	{
		printf("    ");
	}
	if (p->type == INTERNAL)
	{
		printf("I (%d)\n", p->freq);
	}
	else
	{
		printf("%c (%d)\n", p->x, p->freq);
	}
	printHuffmanTree(p->left, level + 1);
}

void writecode(char ch, FILE *f)
{
	char *code = getCode(ch);
	total_code_length += strlen(code);
	encoded_message_length++;

	while (*code != '\0')
	{
		if (*code == '1')
			writeBit(1, f);
		else
			writeBit(0, f);
		code++;
	}
}
