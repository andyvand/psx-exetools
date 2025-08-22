#include "stdafx.h"

#define Align(val,align)	(((((val)-1)/(align))+1)*(align))

typedef struct tagCpeHeader
{
	u32 magic:24;	// 'CPE'
	u32 version:8;	// always 1
} CPE_HEADER;

typedef struct tagExeHeader
{
	u8 id[8];		// "PS-X EXE"
	u32 text;		// NULL
	u32 data;		// NULL
	u32 pc0;		// program counter
	u32 gp0;		// global pointer, NULL
	u32 t_addr;		// text address
	u32 t_size;		// text size
	u32 d_addr;		// data address, NULL
	u32 d_size;		// data size, NULL
	u32 b_addr;		// bss address, NULL
	u32 b_size;		// bss size, NULL
	u32 s_addr;		// stack size
	u32 s_size;		// stack address, NULL
	u32 sp,fp,gp,ret,base;	// preserve machine state when going back to parent process, all NULL
} EXE_HEADER;

int StoreFile(LPCTSTR filename, u8* &buffer)
{
	int size;
	buffer=NULL;

	FILE *in=_tfopen(filename,_T("rb+"));

	if(!in) return 0;

	// retrieve size
	fseek(in,0,SEEK_END);
	size=ftell(in);
	// bufferize file
	buffer=new u8[size];
	fseek(in,0,SEEK_SET);
	fread(buffer,size,1,in);

	return size;
}

int _tmain(int argc, _TCHAR* argv[])
{
	_tprintf(_T("CPE2PSX v1.0 - a simple CPE to PS-X EXE converter (Gemini, 2012)\n"));
	if(argc<=1)
	{
		_tprintf(_T("\n  Usage: CPE2PSX input.cpe\n"));
		return 1;
	}

	u8* data;
	int fsize=StoreFile(argv[1],data);

	if(!data || fsize==0)
	{
		_tprintf(_T("Couldn't load \"%s\".\n"),argv[1]);
		return 1;
	}

	CPE_HEADER *h=(CPE_HEADER*)data;

	if(h->magic!=('C'|('P'<<8)|('E'<<16)) && h->version!=1)
	{
		_tprintf(_T("Not a valid CPE file.\n"));
		return 1;
	}

	EXE_HEADER exe_header={0};
	u8* dest=new u8[2*1024*1024];	// 2 MB buffer for the exe
	memset(dest,0,2*1024*1024);

	u32 low=0x80800000;		// highest RAM address (takes into account 8MB ram)
	u32 high=0x80000000;	// lowest RAM address

	// setup PS-X EXE static values
	memcpy(exe_header.id,"PS-X EXE",sizeof(exe_header.id));
	exe_header.s_addr=0x801FFFF0;

	for(int i=sizeof(CPE_HEADER); i<fsize;)
	{
		switch(data[i++])
		{
			// End of file
			case 0:
				break;
			// text segment
			case 1:
				{
					u32 address=*((u32*)&data[i]);
					u32 size=*((u32*)&data[i+4]);
					i+=8;

					// fix RAM addresses (really needed?)
					//address = (address&0x80000000 ? address-0x80000000 : address);

					// update lo-hi ranges when necessary
					if(address < low) low=address;
					if(address+size > high) high=address+size;

					// copy segment to exe buffer
					memcpy(&dest[address-0x80000000],&data[i],size);
					i+=size;
				}
				break;
			// Run address
			case 2:
				i+=4;	//	Address
				break;
			// Set reg x to u32 y
			case 3:
				{
					u16 reg=*((u16*)&data[i]);		//	Register
					u32 val=*((u32*)&data[i+2]);	//	Value
					switch(reg)
					{
					// assign pc0
					case 0x90:
						exe_header.pc0=val;
						break;
					default:
						_tprintf(_T("Unknown register value %d=%x\n"),reg,val);
					}
					i+=6;
				}
				break;
			// Set reg x to u16 y
			case 4:
				i+=2;	//	Register
				i+=2;	//	Value
				break;
			// Set reg x to u8 y
			case 5:
				i+=2;	//	Register
				i++;	//	Value
				break;
			// Set reg x to u24 y
			case 6:
				i+=2;	//	Register
				i+=3;	//	Value
				break;
			// Select workspace
			case 7:
				i+=4;
				break;
			// Select unit (skip, it's always 0)
			case 8:
				i++;
				break;
			default:
				_tprintf(_T("Unknown tag"));
		}
	}

	exe_header.t_addr=low;
	exe_header.t_size=Align(high-low,2048);

	u8 sector[2048]={0};
	memcpy(sector,&exe_header,sizeof(exe_header));
#ifdef JAPAN
	strcpy((char*)&sector[sizeof(exe_header)],"Sony Computer Entertainment Inc. for Japan area");
#elif defined(USA)
    strcpy((char*)&sector[sizeof(exe_header)],"Sony Computer Entertainment Inc. for North America area");
#else /* EUROPE */
    strcpy((char*)&sector[sizeof(exe_header)],"Sony Computer Entertainment Inc. for Europe area");
#endif

	//GString out_name(argv[1]);
#ifdef UNICODE
	std::wstring out_name=argv[1];
#else
	std::string out_name=argv[1];
#endif
	int dot_pos=(int)out_name.rfind(_T('.'));
	// replace extension with ".psx"
	if(dot_pos!=-1)
	{
		out_name=out_name.substr(0,dot_pos);
		out_name+=_T(".psx");
		// make sure names aren't the same
		if(out_name.compare(argv[1])==0)
			out_name+=_T("0");
	}
	// simply append ".psx" if there's no extension
	else out_name+=_T(".psx");

	FILE *out=_tfopen(out_name.c_str(),_T("wb+"));
	fwrite(sector,sizeof(sector),1,out);
	fwrite(&dest[low-0x80000000],Align(high-low,2048),1,out);
	fclose(out);

	delete[] dest;

	return 0;
}

