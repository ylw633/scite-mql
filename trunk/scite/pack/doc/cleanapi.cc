/*
 A small utility to remove those nasty "__P(" macros and comments
 from the std libc func prototypes in the api files used by SciTE
 for calltips and autocompletion...

 --Modified on 29/04/2000 to remove #if macros from inside Xlib.h's
 --function prototypes

 #include &lt;std_disclaimer.h&gt;

   "I do not accept responsibility for any effects, adverse or otherwise,
    that this code may have on you, your computer, your sanity, your dog,
    and anything else that you can think of. Use it at your own risk."

 Usage:
    cleanapi &lt;in.api&gt; &lt;out.api&gt; [OPTIONS]

    Options: -c -&gt; Remove Comments
             -v -&gt; Be (frustratingly :) verbose..

 By Deepak S.
 deepaks@operamail.com
*/

#include &lt;fstream.h&gt;
#include &lt;string.h&gt;

int main(int argc, char *argv[])
{
	ifstream inapi;
	ofstream outapi;

	bool verbose=false, comments=false, comrem=false;

	char aline[512]="", modline[512]="";
	char *pptr, *cptr, *wdptr;

	int modnum, i, parcnt=0;

	if(argc&lt;3)
	{
		cout&lt;&lt;"Usage: cleanapi &lt;in.api&gt; &lt;out.api&gt;"&lt;&lt;endl;
		return -1;
	}

	inapi.open(argv[1]);
	if(!inapi)
	{
		cout&lt;&lt;"Unable to open input file "&lt;&lt;argv[1]&lt;&lt;endl;
		return -1;
	}

	outapi.open(argv[2]);
	if(!outapi)
	{
		cout&lt;&lt;"Unable to open output file "&lt;&lt;argv[2]&lt;&lt;endl;
		return -1;
	}

	if(argc&gt;3)
	{
		for(i=3; i&lt;argc; i++)
			if(!strcmp(argv[i], "-v"))
				verbose=true;
			else if(!strcmp(argv[i], "-c"))
				comments=true;
			else
			{
				cerr&lt;&lt;"Bad option!"&lt;&lt;endl;
				return -1;
			}
	}

	while(inapi.getline(aline, 511)!=NULL)
	{
		modnum=0;

		if(comments)
		{
			comrem=false;
			pptr=NULL;
			wdptr=aline;
			while(wdptr &amp;&amp; (pptr=strstr(wdptr, "/*"))!=NULL)
			{
				cptr=wdptr;
				while(cptr!=pptr)
					modline[modnum++]=*(cptr++);

				if((pptr=strstr(wdptr, "*/"))!=NULL)
					wdptr=pptr+2;

				comrem=true;
			}

			while(*wdptr)
				modline[modnum++]=*(wdptr++);

			modline[modnum]='\0';
			if(verbose &amp;&amp; comrem)
				cout&lt;&lt;"Removed comment(s): \n\""&lt;&lt;aline&lt;&lt;"\" becomes \n\""&lt;&lt;modline&lt;&lt;"\""&lt;&lt;endl;
			strcpy(aline, modline);
			modnum=0;
		}

		if((pptr=strstr(aline, "#if"))!=NULL)
		{
			//Fooling around with pointers is a nice time-pass :)
			cptr=aline;
			while(cptr!=pptr)
				modline[modnum++]=*(cptr++);
			if((cptr=strchr(cptr, ' ')))	//Skip the conditions
			{
				cptr++;
				cptr=strchr(cptr, ' ');
				cptr++;
			}
			pptr=strstr(pptr,"#endif");
			while(cptr!=pptr)
				modline[modnum++]=*(cptr++);
			pptr+=6;
			while(*pptr)
				modline[modnum++]=*(pptr++);
			modline[modnum]='\0';
			if(verbose)
				cout&lt;&lt;"Removed nasty #if: \n\""&lt;&lt;aline&lt;&lt;"\" becomes \n\""&lt;&lt;modline&lt;&lt;"\""&lt;&lt;endl;
			strcpy(aline, modline);
			modnum=0;
		}

		if((pptr=strstr(aline, " __P("))!=NULL)
		{
			parcnt=1;
			cptr=aline;
			while(cptr!=pptr)
				modline[modnum++]=*(cptr++);
			pptr+=4;
			while(parcnt &amp;&amp; *pptr)
			{
				pptr++;
				if(*pptr=='(') ++parcnt;
				if(*pptr==')') --parcnt;
			}
			cptr+=5;
			while(cptr!=pptr)
				modline[modnum++]=*(cptr++);
			if(*(cptr+1))
			{
				cptr++;
				while(*cptr)
					modline[modnum++]=*(cptr++);
			}
			modline[modnum]='\0';
			if(verbose)
				cout&lt;&lt;"Removed nasty macro: \n\""&lt;&lt;aline&lt;&lt;"\" becomes \n\""&lt;&lt;modline&lt;&lt;"\""&lt;&lt;endl;
			strcpy(aline, modline);
		}

		outapi&lt;&lt;aline;
		outapi&lt;&lt;endl;
	}

	inapi.close();
	outapi.close();
	cout&lt;&lt;"Finished cleaning "&lt;&lt;argv[1]&lt;&lt;" ..."&lt;&lt;endl;

	return 0;
}
