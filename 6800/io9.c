/* Small-C Input-Output Library for FLEX9 */

putchar(c)
         char c;
{
#asm
 FCB 86
 LDA 3,S
 CMPA #13
 BNE :1
 JSR $CD24
 BRA :2
:1 JSR $CD18
:2 LDD 2,S
 JMP RTSC
#endasm
}

getchar()
{
#asm
 FCB 86
 JSR $CD15
 CMPA #13
 BNE :1
 JSR $CD24
 LDA #13
:1 TFR A,B
 CLRA
 JMP RTSC
#endasm
}

puts(s)
         char s[];
{
         int k;
         k=0;
         while(putchar(s[k++]));
}
