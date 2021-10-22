/*
DECISIONI VARIE:
|entity|
cod 0: vuota
cod 1: pipotto del giocatore------------31: blu scuro
cod 2: pipotto nemico-------------------79: rosso scuro
cod 3: proiettile-----------------------15: nero
cod 4: scia proiettile del giocatore----159: blu bello
cod 5: scia del proiettile nemico-------207: rosso bello
cod 6: pipotto nemico colpito-----------239: giallo
cod 7: pipotto giocatore colpito--------191: ciano
*/

#include<iostream>
#include<stdlib.h>
#include<windows.h>
#include<cstring>
#include<stdio.h>
#include <time.h>
#define random(x) rand() % x
#define randomize srand((unsigned)time(NULL))
using namespace std;
//variabili globali
int entity[69][33];                          //matrice per campo
int npipotti, coordpipotti[20][2];          //vettore per memorizzare coordinate pipotti (il loro centro)
int coordproiettili[100][12];    //vettore per memorizzare i proiettili (0,1 coord bersaglio; 2,3 coord proiettile; 4,5, 6,7, 8,9, 10, 11 coord scia)
int traiettoriaproiettili[100][102]; //il 100° posto e' riservato alla presenza o meno di un proiettile: se c'e' 0 non vi e' salvato alcun proiettile, se c'e' 1 c'e' salvato un proiettile; il 101° e' per lo stato della coda
int vita; //vita del boss da 0 a 23

//SOTTOPROGRAMI
//PROCEDURA che GENERA numero CASUALE
int GeneraNumeriCasuali(int max)
{
    return (random(max)+1);
}

//colori
void SetColor(unsigned short color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE),color);
}

//procedura che inizializza lo stato di tutte le celle della matrice ent a 0 (vuote)
void inizent(int ent[69][33])
{
    for(int x=0; x<69; x++)
    {
        for(int y=0; y<33; y++)
        {
            ent[x][y]=0;
        }
    }
}

//procedura che inizializza/annulla la posizione di un pipotto
void inizpipotto(int ent[69][33], int cpip[20][2], int npip, int stato)
{
    for(int col=cpip[npip][0]-1; col<=cpip[npip][0]+1; col++)
    {
        for(int rig=cpip[npip][1]-1; rig<=cpip[npip][1]+1; rig++)
        {
            ent[col][rig]=stato;
        }
    }
    if(stato==0) cpip[npip][0]=-1;   //RICORDATI DI CIOOHH
}

void inizboss(int ent[69][33], int cpip[20][2], int npip, int stato)
{
    for(int col=cpip[npip][0]-4; col<=cpip[npip][0]+4; col++)
    {
        for(int rig=cpip[npip][1]-4; rig<=cpip[npip][1]+4; rig++)
        {
            ent[col][rig]=stato;
        }
    }
    if(stato==0) cpip[npip][0]=-1;   //RICORDATI DI CIOOHH
}

void inizcoordpipotti(int cpip[20][2])
{
    for(int npip=0; npip<20; npip++)
    {
        cpip[npip][0]=-1;
    }
}

int pipottovuoto(int cpip[20][2])
{
    for(int npip=2; npip<20; npip++) //parte da 2 perche' la 0 e' per il giocatore e la 1 e' per i boss
    {
        if(cpip[npip][0]==-1) return npip;
    }
    return 2;
}

bool nessunpipotto(int cpip[20][2])
{
    for(int npip=1; npip<20; npip++)
    {
        if(cpip[npip][0]!=-1) return false;
    }
    return true;
}

//procedura che inizializza la matrice traiettoria a 0
void iniztraiettoria(int tp[100][102])
{
    for(int np=0; np<100; np++)
    {
        for(int s=0; s<102; s++)
        {
            tp[np][s]=0;
        }
    }
}

//cancellazione di un proiettile
void azzeraproiettile(int ent[69][33], int cp[100][12], int tp[100][102], int np)
{
    //azzeramento traiettoria
    for(int i=0; i<102; i++)
    {
        tp[np][i]=0;
    }

    //azzeramento entity
    ent[cp[np][2]][cp[np][3]]=0;
    ent[cp[np][4]][cp[np][5]]=0;
    ent[cp[np][6]][cp[np][7]]=0;
    ent[cp[np][8]][cp[np][9]]=0;
    ent[cp[np][10]][cp[np][11]]=0;

    //Non serve modificare la cp perche' per la visualizzazione serve che tp[np][100] sia 1 e perche' essa viene modificata senza creare danni quando viene generato un altro proiettile
}

//inizializzzione di tutte le celle del proiettile al centro del pipotto che lo ha sparato
void inizproiettile(int pip[20][2], int cp[100][12], int npip, int np)
{
    for(int i=2; i<12; i++)
    {
        cp[np][i]=pip[npip][i%2];
    }
}

//funzione che trova unospazio per memorizzare un nuovo proiettile
int proiettilevuoto(int tp[100][102])
{
    for(int i=0; i<100; i++)
    {
        if(tp[i][100]==0) return i;
    }
    return 0; //se sono tutte piene cancella il proiettile di indice 0
}

//procedura che srive i numeri dell'asse delle colonne
void scrivinumcol()
{
    for(int i=0; i<23; i++)
    {
        if(i<9) cout<<"     "<<i+1;
        else cout<<"    "<<i+1;
    }
    cout<<"    "<<endl;
}

//procedura che scrive i numeri della colonna a sinistra del campo di gioco (differenzia i numeri a doppia cifra da quelli a singola)
void scrivinumrig(int rig)
{
    if(rig<10) cout<<rig<<" ";
    else cout<<rig;
}

//procedura che permette alla procedure del campo di gioco di avere una colorazione a scacchiera
void sceglicolore(int rig, int col)
{
    if((rig%2!=0 && col%2!=0) || (rig%2==0 && col%2==0)) SetColor(127); //grigio chiaro
    else SetColor(143); //grigio scuro
}

//procedura che, dato un colore, disegna 4 spazi di quel colore (usato per i codici 1, 2, 4) per poi reimpostare il colore esatto della scacchiera
//175=verde 143=grigio
void impostacolore(int cod, int rig, int col)
{
    SetColor(cod);
    cout<<"  ";
    sceglicolore(rig, col);
}

//da modificare
void ctrlstatocella(int ent[69][33], int rig, int col)
{
    if(ent[col][rig]==0)
    {
        sceglicolore(rig/3, col/3);   //sta roba del diviso 3 sfrutta gli arrotondamenti: 0/3=0, 1/3=0, 2/3=0 (le prime tre celle), MA 3/3=1, 4/3=1, 5/3=1
        cout<<"  ";
    }
    else
    {
        if(ent[col][rig]==4) impostacolore(159, rig, col);
        else
        {
            if(ent[col][rig]==2) impostacolore(79, rig, col);
            else
            {
                if(ent[col][rig]==5) impostacolore(207, rig, col);
                else
                {
                    if(ent[col][rig]==3) impostacolore(15, rig, col);
                    else
                    {
                        if(ent[col][rig]==1) impostacolore(31, rig, col);
                        else
                        {
                            if(ent[col][rig]==6) impostacolore(239, rig, col);
                            else
                            {
                                if(ent[col][rig]==7) impostacolore(191, rig, col);
                            }
                        }
                    }
                }
            }
        }
    }
}

//procedura che manda a video il campo da gioco con pipotti, proiettili e scie
void campo(int ent[69][33])
{
    cout<<endl<<"  ";
    SetColor(241);
    cout<<"                                                                                                                                              "<<endl;
    for(int rig=32; rig>=0; rig--)
    {
        SetColor(1);
        cout<<"  ";
        SetColor(250);
        if(rig%3==1) scrivinumrig((rig-1)/3+1);
        else cout<<"  ";
        for(int col=0; col<69; col++)
        {
            ctrlstatocella(ent, rig, col);
        }
        SetColor(241);
        cout<<"  ";
        cout<<endl;
    }
    SetColor(15);
    cout<<"  ";
    SetColor(251);
    cout<<"     A     B     C     D     E     F     G     H     I     J     K     L     M     N     O     P     Q     R     S     T     U     V     W    ";
    SetColor(15);
    cout<<endl;
}

void barravita(int vita, bool boss)
{
    int sup=vita;
    if(boss==false) cout<<endl<<endl<<endl<<endl;
    else
    {
        SetColor(1);
        cout<<endl<<"  ";
        SetColor(241);
        cout<<"                                                                                                                                              ";
        for(int k=0; k<2; k++)
        {
            SetColor(1);
            cout<<endl;
            cout<<"  ";
            SetColor(241);
            cout<<"  ";
            for(int i=0; i<23; i++)
            {
                if(sup>0)
                {
                    SetColor(207);
                    if(k==0) cout<<"      ";
                    else cout<<"      ";
                    sup--;
                }
                else
                {
                    SetColor(1);
                    cout<<"      ";
                }
            }
            SetColor(241);
            cout<<"  ";
            sup=vita;
        }
        SetColor(1);
    }
}

int numerodir(char let)
{
    if(let=='W' || let=='w') return 1;
    if(let=='A' || let=='a') return 2;
    if(let=='S' || let=='s') return 3;
    if(let=='D' || let=='d') return 4;
    if(let=='Z' || let=='z') return 5;
    if(let=='X' || let=='x') return 6;
    return let;
}

int chiedimossa()
{
    char m;
    do
    {
        cout<<"Inserisci mossa (w,a,s,d: movimento; z: sparo; x: fermo): ";
        cin>>m;
    }while(m!='w' && m!='W' && m!='a' && m!='A' && m!='s' && m!='S' && m!='d' && m!='D' && m!='x' && m!='X' && m!='z' && m!='Z');
    return numerodir(m);
}

bool ctrlsovrapposizione(int cpip[20][2], int npip1)
{
    for(int npip2=0; npip2<20; npip2++)
    {
        if(npip1!=npip2 && cpip[npip2][0]!=-1 && cpip[npip1][0]==cpip[npip2][0] && cpip[npip1][1]==cpip[npip2][1]) return true;
    }
    return false;
}

void traducidir(int dir, int &x, int &y, bool &errore, int area)
{
    int dist;
    if(area==4) dist=6;
    else dist=3;

    if(dir==3)
    {
        if(y-dist>=1)
        {
            y=y-3;
        }
        else errore=true;
    }
    if(dir==2)
    {
        if(x-dist>=1)
        {
            x=x-3;
        }
        else errore=true;
    }
    if(dir==1)
    {
        if(y+dist<=31)
        {
            y=y+3;
        }
        else errore=true;
    }
    if(dir==4)
    {
        if(x+dist<=67)
        {
            x=x+3;
        }
        else errore=true;
    }
}

bool spostamento(int ent[69][33], int cpip[20][2], int npip, int dir, bool giocatore)
{
    bool errore=false;
    int recx=cpip[npip][0], recy=cpip[npip][1], area;

    if(npip==1) area=4;
    else area=1;

    traducidir(dir, cpip[npip][0], cpip[npip][1], errore, area);
    if(errore==false && ctrlsovrapposizione(cpip, npip)==false)
    {
        //annullamento posizione precedente
        for(int col=recx-area; col<=recx+area; col++)
        {
            for(int rig=recy-area; rig<=recy+area; rig++)
            {
                ent[col][rig]=0;
            }
        }
        //spostamento
        for(int col=cpip[npip][0]-area; col<=cpip[npip][0]+area; col++)
        {
            for(int rig=cpip[npip][1]-area; rig<=cpip[npip][1]+area; rig++)
            {
                if(npip==0) ent[col][rig]=1;
                else ent[col][rig]=2;
            }
        }
        return true;
    }
    else
    {
        cpip[npip][0]=recx;
        cpip[npip][1]=recy;
        if(giocatore==true) cout<<"Non puoi spostarti qui"<<endl;
        return false;
    }
}

//traduzione dal char delle colonne (per l'utente) a int (per il computer). Se l'utente ha inserito un valore diverso da quelli contemplati, la funzione ritorna -1.
int traducichar1(char a)
{
    if(a=='a' || a=='A') return 1;
    if(a=='b' || a=='B') return 4;
    if(a=='c' || a=='C') return 7;
    if(a=='d' || a=='D') return 10;
    if(a=='e' || a=='E') return 13;
    if(a=='f' || a=='F') return 16;
    if(a=='g' || a=='G') return 19;
    if(a=='h' || a=='H') return 22;
    if(a=='i' || a=='I') return 25;
    if(a=='j' || a=='J') return 28;
    if(a=='k' || a=='K') return 31;
    if(a=='l' || a=='L') return 34;
    if(a=='m' || a=='M') return 37;
    if(a=='n' || a=='N') return 40;
    if(a=='o' || a=='O') return 43;
    if(a=='p' || a=='P') return 46;
    if(a=='q' || a=='Q') return 49;
    if(a=='r' || a=='R') return 52;
    if(a=='s' || a=='S') return 55;
    if(a=='t' || a=='T') return 58;
    if(a=='u' || a=='U') return 61;
    if(a=='v' || a=='V') return 64;
    if(a=='w' || a=='W') return 67;
    return -1;
}

//traduzione del char delle righe (con una sola cifra) in int. Nonostante le righe siano codificate da un numero, esso è di tipo char. Ciò per evitare malfunzionamenti dovuti al controllo dell'input se l'utente al posto di un int mettesse un char
int traducichar2(char a)
{
    if(a=='1') return 1;
    if(a=='2') return 4;
    if(a=='3') return 7;
    if(a=='4') return 10;
    if(a=='5') return 13;
    if(a=='6') return 16;
    if(a=='7') return 19;
    if(a=='8') return 22;
    if(a=='9') return 25;
    return -1;
}

//traduzione da char a int con numeri di due cifre
int traducichar3(char a[4])
{
    if(a[0]=='1' && a[1]=='0') return 28;
    if(a[0]=='1' && a[1]=='1') return 31;
    return -1;
}

int trasformax(char x[4])
{
    if(strlen(x)==1) return traducichar1(x[0]);
    return -1;
}

//funzione che traduce la stringa che contiene l'ordinata (numero) in un int. Distingue singola cifra e doppia cifra
int trasformay(char y[4])
{
    if(strlen(y)==1) return traducichar2(y[0]);
    if(strlen(y)==2) return traducichar3(y);
    return -1;
}

void chiedisparo(int &x, int &y)
{
    char charx[4], chary[4];
    do
    {
        cout<<"Scegli direzione(indica una cella): "<<endl;
        do
        {
            cout<<"x: ";
            fflush(stdin);
            gets(charx);
            x=trasformax(charx); //traduzione da char a int
        }while(x<0); //solo <0 perchè la trasformax ritorna -1 se l'input non è valido
        do
        {
            cout<<"y: ";
            fflush(stdin);
            gets(chary);
            y=trasformay(chary);
        }while(y<0);
    }while(0);
}

void proiettasparo(int &bersx, int &bersy, int partx, int party) //PER ORA NON LA USIAMO (da usare solo se non ti interssa essere troppo preciso per il passaggio sul bersaglio effettivo (la figata sarebbe che il proiettile continua fino alla parete))
{
    do
    {
        bersx=2*bersx-partx; //inverso della formula del punto medio
        bersy=2*bersy-party;
    }while(bersx>=0 && bersx<69 && bersy>=0 && bersy<33);
}

int ass(int n)
{
    if(n<0) return -n;
    return n;
}

void calcolacateti(int xa, int ya, int xb, int yb, int &deltax, int &deltay)
{
    deltax=xa-xb;
    deltay=ya-yb;
}

void salvapercorso(int tp[100][102], int np, int n, int iniz, int inc, int part)
{
    //cout<<endl<<endl;
    int i;
    tp[np][0]=0;    //RICORDATI DI STA ROBA QUANDO FAI I CONTROLLIIIHHHHHH
    for(i=part; i<=n; i=i+2)
    {
        tp[np][i]=iniz;
        //cout<<i<<": "<<tp[np][i]<<endl;
        tp[np][i+1]=inc;
        //cout<<i+1<<": "<<tp[np][i+1]<<endl;
    }
    tp[np][i-1]=0;
    //cout<<i-1<<": "<<tp[np][i-1]<<endl;
}

void allungacoppie(int tp[100][102], int np, int &diff, int su, int giu, int delta)
{
    while(diff!=0)
    {
        if(delta>0)
        {
            tp[np][su]++;
            tp[np][giu]++;
        }
        else
        {
            tp[np][su]--;
            tp[np][giu]--;
        }
        diff=diff-2;
        su=su+2;
        giu=giu-2;
    }
}

void allungasegmenti(int tp[100][102], int np, int centr, int &diff, int delta)
{
    if(diff%2!=0) //se la differenza e' dispari
    {
        //incrementa (o decrementa) il centrale e decrementa la differenza
        if(delta>0) tp[np][centr]++;
        else tp[np][centr]--;
        diff--;
    }
    allungacoppie(tp, np, diff, centr+2, centr-2, delta);
}

void strutturapercorso(int tp[100][102], int np, int delta1, int delta2, int incr)
{
    int ns;  //ns=numero di segmenti da fare
    int iniz; //lunghezza di ciascuno dei segmenti in cui e' divisa la distanza maggiore all'inizio del processo(quando sono ancora tutti uguali)
    int diff; //differenza tra la distanza maggiore e la somma delle lunghezze dei segmenti
    int part; //variabile che serve per far partire in modo diverso la salvapercorsose delta1 e' verticale o orizzontale, si stabilisce in base alla incr(che distingue i due casi)
    ns=ass(delta2)+1;  //nc=numero di segmenti da fare //se sei in dubbio guarda su paint ;)
    iniz=delta1/ns;  //cosi' facendo si approssima per difetto
    if(incr==1) part=0; //se il delta maggiore e' x fai partire la salvapercorso dall'indice 0
    else part=1; //se il delta maggiore e' y fai partire da 1
    if(delta2>=0) salvapercorso(tp, np, (2*ass(delta2))+1, iniz, 1, part); //CONTROLLA PER VERTICALE E ORIZZONTALE
    if(delta2<0) salvapercorso(tp, np, (2*ass(delta2))+1, iniz, -1, part);
    diff=ass(delta1)-ass((iniz*ns));
    if(diff!=0) //se c'e' differenza
    {
        if(ns%2!=0) //se i segmenti sono dispari
        {
            ns=ns-incr; //per il centrale
            allungasegmenti(tp, np, ns, diff, delta1);
        }
        else //se i segmenti sono pari
        {
            ns=ns-incr; //per il centrale
            allungasegmenti(tp, np, ns+1, diff, delta1);
        }
    }
}

void tracciapercorso(int cp[100][12], int tp[100][102], int np, int statocoda)
{
    int deltax, deltay; //distanza orizzontale e distanza verticale
    proiettasparo(cp[np][0], cp[np][1], cp[np][2], cp[np][3]);
    calcolacateti(cp[np][0], cp[np][1], cp[np][2], cp[np][3], deltax, deltay);

    //se la distanza orizzontale e' maggiore della verticale
    if(ass(deltax)>ass(deltay))
    {
        strutturapercorso(tp, np, deltax, deltay, 1);
    }
    //se la distanza verticale e' maggiore della orizzontale
    else
    {
        strutturapercorso(tp, np, deltay, deltax, 0);
    }

    tp[np][100]=1;  //segnali che qui c'e' un proiettile
    tp[np][101]=statocoda;
}

void spostaproiettile(int ent[69][33], int cp[100][12], int np, int somx, int somy)
{
    //trascinamento coda
    cp[np][11]=cp[np][9];
    cp[np][10]=cp[np][8];
    cp[np][9]=cp[np][7];
    cp[np][8]=cp[np][6];
    cp[np][7]=cp[np][5];
    cp[np][6]=cp[np][4];
    cp[np][5]=cp[np][3];
    cp[np][4]=cp[np][2];
    //spostamento punta
    cp[np][2]=cp[np][2]+somx;
    cp[np][3]=cp[np][3]+somy;
}

void cambiastatoproiettile(int ent[69][33], int cp[100][12], int np, int stato)
{
    int statopunta;
    if(stato==0) statopunta=0;
    else statopunta=3;
    //cambio stato celle sparo
    if(ent[cp[np][4]][cp[np][5]]!=1 && ent[cp[np][4]][cp[np][5]]!=2 && ((ent[cp[np][4]][cp[np][5]]!=3 && stato!=0) || stato==0)) ent[cp[np][4]][cp[np][5]]=stato;
    if(ent[cp[np][6]][cp[np][7]]!=1 && ent[cp[np][6]][cp[np][7]]!=2 && ((ent[cp[np][6]][cp[np][7]]!=3 && stato!=0) || stato==0)) ent[cp[np][6]][cp[np][7]]=stato;
    if(ent[cp[np][8]][cp[np][9]]!=1 && ent[cp[np][8]][cp[np][9]]!=2 && ((ent[cp[np][8]][cp[np][9]]!=3 && stato!=0) || stato==0)) ent[cp[np][8]][cp[np][9]]=stato;
    if(ent[cp[np][10]][cp[np][11]]!=1 && ent[cp[np][10]][cp[np][11]]!=2 && ((ent[cp[np][10]][cp[np][11]]!=3 && stato!=0) || stato==0)) ent[cp[np][10]][cp[np][11]]=stato;
    //cambio stato punta
    if(ent[cp[np][2]][cp[np][3]]!=1 && ent[cp[np][2]][cp[np][3]]!=2) ent[cp[np][2]][cp[np][3]]=statopunta;
}

void avanza(int ent[69][33], int cp[100][12], int tp[100][102], int np, int somx, int somy, int i)
{
    int sup;
    if(tp[np][i]>0)
    {
        spostaproiettile(ent, cp, np, somx, somy);
        tp[np][i]--; //avvicinalo allo 0
        sup=-1;
    }
    if(tp[np][i]<0)
    {
        spostaproiettile(ent, cp, np, -somx, -somy);
        tp[np][i]++;
        sup=1;
    }
}

void leggipercorso(int ent[69][33], int cp[100][12], int tp[100][102], int np)
{
    int i=0;
    while(tp[np][i]==0 && i<100)
    {
        i++;
    }
    if(i<100)
    {
        //se stai analizzando una x
        if(i%2==0) avanza(ent, cp, tp, np, 1, 0, i);
        //se stai analizzando una y
        else avanza(ent, cp, tp, np, 0, 1, i);
    }
}

void visualtraiettoria(int tp[100][102], int np) //per test
{
    for(int i=0; i<100; i++)
    {
        if(i%2==0) cout<<"deltax "<<i+1<<":"<<tp[np][i]<<endl;
        else cout<<"deltay "<<i+1<<":"<<tp[np][i]<<endl;
    }
}

void sparo(int ent[69][33], int cpip[20][2], int cp[100][12], int tp[100][102], int npip, int np, bool giocatore)
{
    if(giocatore==true) chiedisparo(cp[np][0], cp[np][1]); //chiedi le coordinate del bersaglio all'utente
    else
    {
        //PER ORA prendi come bersaglio la posizione del giocatore
        cp[np][0]=cpip[0][0];
        cp[np][1]=cpip[0][1];
    }
    inizproiettile(cpip, cp, npip, np);
    if(giocatore==true) tracciapercorso(cp, tp, np, 4);
    else tracciapercorso(cp, tp, np, 5);

    //se il proiettile e' lanciato da un boss
    if(npip==1)
    {
        //avanza nel percorso fino a quando esci dal "corpo" del boss
        do
        {
            leggipercorso(ent, cp, tp, np);
        }while(cp[np][2]<=cpip[npip][0]+4 && cp[np][2]>=cpip[npip][0]-4 && cp[np][3]<=cpip[npip][1]+4 && cp[np][3]>=cpip[npip][1]-4);
    }
}

//procedura che stabilisce quali direzioni hanno la priorita' inbase alla posizione del pipotto
void definiscipriorita(int cpip[20][2], int npip, int &dir1, int &dir2, int &dir3, int &dir4)
{
    if(cpip[npip][1]<=16)
    {
        dir1=1;
        if(cpip[npip][0]>34)
        {
            dir2=2;
            dir3=4;
        }
        else
        {
            dir2=4;
            dir3=2;
        }
        dir4=3;
    }
    if(cpip[npip][1]>16)
    {
        dir1=3;
        if(cpip[npip][0]>34)
        {
            dir2=2;
            dir3=4;
        }
        else
        {
            dir2=4;
            dir3=2;
        }
        dir4=1;
    }
    if(cpip[npip][0]<=34)
    {
        dir1=4;
        if(cpip[npip][1]>16)
        {
            dir2=3;
            dir3=1;
        }
        else
        {
            dir2=1;
            dir3=3;
        }
        dir4=2;
    }
    if(cpip[npip][0]>34)
    {
        dir1=2;
        if(cpip[npip][1]>16)
        {
            dir2=3;
            dir3=1;
        }
        else
        {
            dir2=1;
            dir3=3;
        }
        dir4=4;
    }
}

//funzione che ritorna una direzione generata in modo casuale ma con diverse probabilita' per ciascuna direzione
int prioritacasuali(int cpip[20][2], int npip)
{
    int n=GeneraNumeriCasuali(10), dir1, dir2, dir3, dir4;
    definiscipriorita(cpip, npip, dir1, dir2, dir3, dir4);
    if(n>0 && n<=3) return dir1;
    if(n>3 && n<=6) return 5; //sparo
    if(n==7 || n==8) return dir2;
    if(n==9) return dir3;
    if(n==10) return dir4;
}

void mossa(int ent[69][33], int cpip[20][2], int cp[100][12], int tp[100][102], int npip, bool giocatore)
{
    int m;
    bool ok=true;
    do
    {
        if(giocatore==true) m=chiedimossa();
        else m=prioritacasuali(cpip, npip);
        if(m!=5 && m!=6) ok=spostamento(ent, cpip, npip, m, giocatore);
    }while(ok==false);
    if(m==5) sparo(entity, cpip, cp, tp, npip, proiettilevuoto(tp), giocatore);
}

bool pVSpip(int ent[69][33], int cpip[20][2], int cp[100][12], int tp[100][102], int np, int &vita)
{
    int npip=0, ent2[69][33], area;
    inizent(ent2);
    while(npip<20)
    {
        if(cpip[npip][0]!=-1)
        {
            if(npip==1) area=4;
            else area=1;
            if((cp[np][2]<=cpip[npip][0]+area && cp[np][2]>=cpip[npip][0]-area) && (cp[np][3]<=cpip[npip][1]+area && cp[np][3]>=cpip[npip][1]-area)) //se la punta del proiettile e' compresa nell'area 3x3 (o 9x9 per i boss) del pipotto
            {
                if(cp[np][6]==cpip[npip][0] && cp[np][7]==cpip[npip][1]) //se la coda si trova al centro del pipotto (sta roba serve per non far autodistruggere un pipotto quando spara)
                {
                    //non fare niente(sto if serve solo per l'else dopo)
                }
                else
                {
                    if(npip!=1) //se il pipotto colpito non e' un boss
                    {
                        //visualizzazione pipotto colpito con il proiettile che l'ha piato
                        if(npip==0) inizpipotto(ent2, cpip, npip, 7);
                        else inizpipotto(ent2, cpip, npip, 6);
                        cambiastatoproiettile(ent2, cp, np, tp[np][101]);
                        system("cls");
                        campo(ent2);
                        system("pause");

                        //distruzione pipotto e proiettile nelle varabili ufficiali
                        inizpipotto(ent, cpip, npip, 0);
                        azzeraproiettile(ent, cp, tp, np);
                        return false;
                    }
                    else //se e' un boss
                    {
                        vita--;
                        azzeraproiettile(ent, cp, tp, np);
                        if(vita==0)
                        {
                            inizboss(ent, cpip, npip, 6);
                            system("cls");
                            cout<<endl<<endl<<endl<<endl;
                            campo(ent);
                            system("pause");

                            inizboss(ent, cpip, npip, 0);
                        }
                    }
                }
            }
        }
        npip++;
    }
    return true;
}

bool pVSp(int ent[69][33], int cp[100][12], int tp[100][102], int np1)
{
    int np2=0;
    while(np2<100)
    {
        if(tp[np2][100]==1 && np2!=np1 && cp[np1][2]==cp[np2][2] && cp[np1][3]==cp[np2][3])
        {
            azzeraproiettile(ent, cp, tp, np1);
            azzeraproiettile(ent, cp, tp, np2);
            return false;
        }
        np2++;
    }
    return true;
}

bool ctrlproiettile(int ent[69][33], int cpip[20][2], int cp[100][12], int tp[100][102], int np, int &vita)
{
    //controllo per uscita dal campo
    if(cp[np][2]==0 || cp[np][2]==68 || cp[np][3]==0 || cp[np][3]==32)
    {
        azzeraproiettile(ent, cp, tp, np);
        return false;
    }
    //controllo per scontro contro pipotto
    if(pVSpip(ent, cpip, cp, tp, np, vita)==false) return false;
    //controllo per scontro contro proiettile
    if(pVSp(ent, cp, tp, np)==false) return false;
    return true;
}

/*void gestisciproiettili(int ent[69][33], int cpip[20][2], int cp[100][12], int tp[100][102], int vel)
{
    int np=0, i=0;
    do
    {
        while(tp[np][100]==0 && np<100)
        {
            np++;
        }
        if(np<100) //se almeno un proiettile e' presente
        {

            do //ciclo per fare andare il proiettile piu' veloce (in base a vel)
            {
                leggipercorso(ent, cp, tp, np);
                i++;
            }while(ctrlproiettile(ent, cpip, cp, tp, np)==true && i<vel);
            cambiastatoproiettile(ent, cp, np, tp[np][101]);
            i=0;
            np++;  //devi passare all'indice successivo una volta che hai controllato questo proiettile
        }
    }while(np<100); //fallo per tutti i proiettili presenti
}*/

void gestisciproiettili(int ent[69][33], int cpip[20][2], int cp[100][12], int tp[100][102], int vel, int &vita)
{
    int np=0, i=0;
    bool cestino;
    do
    {
        do
        {
            while(tp[np][100]==0 && np<100)
            {
                np++;
            }
            if(np<100) //se almeno un proiettile e' presente
            {
                leggipercorso(ent, cp, tp, np);
                cestino=ctrlproiettile(ent, cpip, cp, tp, np, vita);
                np++;  //devi passare all'indice successivo una volta che hai controllato questo proiettile
            }
        }while(np<100); //fallo per tutti i proiettili presenti
        i++;
        np=0;
    }while(i<vel); //ciclo per fare andare il proiettile piu' veloce (in base a vel)

    do
    {
        while(tp[np][100]==0 && np<100)
        {
            np++;
        }
        if(np<100) //se almeno un proiettile e' presente
        {
            cambiastatoproiettile(ent, cp, np, tp[np][101]);
            np++;  //devi passare all'indice successivo una volta che hai controllato questo proiettile
        }
    }while(np<100); //fallo per tutti i proiettili presenti
}


void gestiscipipotti(int ent[69][33], int cpip[20][2], int cp[100][12], int tp[100][102])
{
    int npip=1;
    do
    {
        while(cpip[npip][0]==-1 && npip<20)
        {
            npip++;
        }
        if(npip<20)
        {
            mossa(ent, cpip, cp, tp, npip, false);
            //pezzo di codice che autodistrugge un pipotto se esso coincide con un altro pipotto
            for(int i=0; i<20; i++)
            {
                if(i!=npip && cpip[npip][0]==cpip[i][0] && cpip[npip][1]==cpip[i][1])
                {
                    cpip[npip][0]=-1;
                }
            }
            npip++;
        }
    }while(npip<20);
}

void cancalldabullets(int ent[69][33], int cp[100][12], int tp[100][102])
{
    for(int np=0; np<100; np++)
    {
        if(tp[np][100]==1) cambiastatoproiettile(ent, cp, np, 0);
    }
}

void zeroalldabullets(int ent[69][33], int cp[100][12], int tp[100][102])
{
    for(int np=0; np<100; np++)
    {
        if(tp[np][100]==1) azzeraproiettile(ent, cp, tp, np);
    }
}

void pipottipack1(int ent[69][33], int cpip[20][2])
{
    //posizione pipotti
    int npip=pipottovuoto(cpip);
    cpip[npip][0]=67;
    cpip[npip][1]=1;
    inizpipotto(ent, cpip, npip, 2);

    npip=pipottovuoto(cpip);
    cpip[npip][0]=1;
    cpip[npip][1]=1;
    inizpipotto(ent, cpip, npip, 2);

    npip=pipottovuoto(cpip);
    cpip[npip][0]=67;
    cpip[npip][1]=31;
    inizpipotto(ent, cpip, npip, 2);

    npip=pipottovuoto(cpip);
    cpip[npip][0]=1;
    cpip[npip][1]=31;
    inizpipotto(ent, cpip, npip, 2);

    //posizione giocatore
    coordpipotti[0][0]=34;
    coordpipotti[0][1]=16;
    inizpipotto(entity, coordpipotti, 0, 1);
}

void pipottipack2(int ent[69][33], int cpip[20][2])
{
    int npip=pipottovuoto(cpip);
    cpip[npip][0]=34;
    cpip[npip][1]=1;
    inizpipotto(ent, cpip, npip, 2);

    npip=pipottovuoto(cpip);
    cpip[npip][0]=34;
    cpip[npip][1]=31;
    inizpipotto(ent, cpip, npip, 2);

    npip=pipottovuoto(cpip);
    cpip[npip][0]=67;
    cpip[npip][1]=16;
    inizpipotto(ent, cpip, npip, 2);

    npip=pipottovuoto(cpip);
    cpip[npip][0]=1;
    cpip[npip][1]=16;
    inizpipotto(ent, cpip, npip, 2);

    npip=pipottovuoto(cpip);
    cpip[npip][0]=40;
    cpip[npip][1]=16;
    inizpipotto(ent, cpip, npip, 2);

    npip=pipottovuoto(cpip);
    cpip[npip][0]=28;
    cpip[npip][1]=16;
    inizpipotto(ent, cpip, npip, 2);

    //posizione gocatore
    inizpipotto(ent, cpip, 0, 0);
    cpip[0][0]=34;
    cpip[0][1]=16;
    inizpipotto(ent, cpip, 0, 1);
}

void pipottipack3(int ent[69][33], int cpip[20][2])
{
    //posizione boss
    cpip[1][0]=34;
    cpip[1][1]=16;
    inizboss(ent, cpip, 1, 2);

    //posizione gocatore
    inizpipotto(ent, cpip, 0, 0);
    cpip[0][0]=34;
    cpip[0][1]=1;
    inizpipotto(ent, cpip, 0, 1);
}

void quad(int ent[69][33], int x, int y, int stato)
{
    for(int i=x-1; i<=x+1; i++)
    {
        for(int k=y-1; k<=y+1; k++)
        {
            ent[i][k]=stato;
        }
    }
}

void vittoria(int ent[69][33])
{
    cout<<endl<<endl<<endl<<endl;
    inizent(ent);

    //H
    quad(ent, 22, 19, 1);
    quad(ent, 22, 22, 1);
    quad(ent, 22, 25, 1);
    quad(ent, 22, 28, 1);

    quad(ent, 25, 22, 1);

    quad(ent, 28, 19, 1);
    quad(ent, 28, 22, 1);
    quad(ent, 28, 25, 1);
    quad(ent, 28, 28, 1);

    //A
    quad(ent, 34, 19, 1);
    quad(ent, 34, 22, 1);
    quad(ent, 34, 25, 1);

    quad(ent, 37, 28, 1);
    quad(ent, 37, 22, 1);

    quad(ent, 40, 19, 1);
    quad(ent, 40, 22, 1);
    quad(ent, 40, 25, 1);

    //I
    quad(ent, 46, 19, 1);
    quad(ent, 46, 22, 1);
    quad(ent, 46, 25, 1);
    quad(ent, 46, 28, 1);

    //V
    quad(ent, 7, 7, 1);
    quad(ent, 7, 10, 1);
    quad(ent, 7, 13, 1);

    quad(ent, 10, 4, 1);

    quad(ent, 13, 4, 1);

    quad(ent, 16, 7, 1);
    quad(ent, 16, 10, 1);
    quad(ent, 16, 13, 1);

    //I
    quad(ent, 22, 13, 1);
    ent[22][10]=4;
    ent[22][9]=4;
    ent[22][8]=4;
    ent[22][7]=4;
    ent[22][6]=3;
    quad(ent, 22, 4, 1);

    //N
    quad(ent, 28, 4, 1);
    quad(ent, 28, 7, 1);
    quad(ent, 28, 10, 1);
    quad(ent, 28, 13, 1);

    quad(ent, 31, 10, 1);

    quad(ent, 34, 7, 1);

    quad(ent, 37, 4, 1);
    quad(ent, 37, 7, 1);
    quad(ent, 37, 10, 1);
    quad(ent, 37, 13, 1);

    //T
    quad(ent, 43, 13, 1);

    quad(ent, 46, 4, 1);
    quad(ent, 46, 7, 1);
    quad(ent, 46, 10, 1);
    quad(ent, 46, 13, 1);

    quad(ent, 49, 13, 1);

    //O
    quad(ent, 55, 4, 1);
    quad(ent, 55, 7, 1);
    quad(ent, 55, 10, 1);
    quad(ent, 55, 13, 1);

    quad(ent, 58, 4, 1);
    quad(ent, 58, 13, 1);

    quad(ent, 61, 4, 1);
    quad(ent, 61, 7, 1);
    quad(ent, 61, 10, 1);
    quad(ent, 61, 13, 1);

    //
    campo(ent);

    cout<<endl;
    system("pause");
}

void sconfitta(int ent[69][33])
{
    cout<<endl<<endl<<endl<<endl;
    inizent(ent);
    //H
    quad(ent, 22, 19, 2);
    quad(ent, 22, 22, 2);
    quad(ent, 22, 25, 2);
    quad(ent, 22, 28, 2);

    quad(ent, 25, 22, 2);

    quad(ent, 28, 19, 2);
    quad(ent, 28, 22, 2);
    quad(ent, 28, 25, 2);
    quad(ent, 28, 28, 2);

    //A
    quad(ent, 34, 19, 2);
    quad(ent, 34, 22, 2);
    quad(ent, 34, 25, 2);

    quad(ent, 37, 28, 2);
    quad(ent, 37, 22, 2);

    quad(ent, 40, 19, 2);
    quad(ent, 40, 22, 2);
    quad(ent, 40, 25, 2);

    //I
    quad(ent, 46, 28, 2);
    ent[46][25]=5;
    ent[46][24]=5;
    ent[46][23]=5;
    ent[46][22]=5;
    ent[46][21]=3;
    quad(ent, 46, 19, 2);

    //P
    quad(ent, 10, 4, 2);
    quad(ent, 10, 7, 2);
    quad(ent, 10, 10, 2);
    quad(ent, 10, 13, 2);

    quad(ent, 13, 7, 2);
    quad(ent, 13, 13, 2);

    quad(ent, 16, 10, 2);

    //E
    quad(ent, 22, 4, 2);
    quad(ent, 22, 7, 2);
    quad(ent, 22, 10, 2);
    quad(ent, 22, 13, 2);

    quad(ent, 25, 4, 2);
    ent[24][8]=2;
    ent[24][9]=2;
    ent[25][8]=2;
    ent[25][9]=2;
    ent[26][8]=2;
    ent[26][9]=2;
    quad(ent, 25, 13, 2);

    //R
    quad(ent, 31, 4, 2);
    quad(ent, 31, 7, 2);
    quad(ent, 31, 10, 2);
    quad(ent, 31, 13, 2);

    quad(ent, 34, 7, 2);
    quad(ent, 34, 13, 2);

    quad(ent, 37, 4, 2);
    quad(ent, 37, 10, 2);

    //S
    quad(ent, 43, 4, 2);
    quad(ent, 43, 10, 2);
    quad(ent, 43, 13, 2);

    quad(ent, 46, 4, 2);
    quad(ent, 46, 7, 2);
    quad(ent, 46, 13, 2);

    //O
    quad(ent, 52, 4, 2);
    quad(ent, 52, 7, 2);
    quad(ent, 52, 10, 2);
    quad(ent, 52, 13, 2);

    quad(ent, 55, 4, 2);
    quad(ent, 55, 13, 2);

    quad(ent, 58, 4, 2);
    quad(ent, 58, 7, 2);
    quad(ent, 58, 10, 2);
    quad(ent, 58, 13, 2);

    //
    campo(ent);

    cout<<endl;
    system("pause");
}

void gestiscipack(int ent[69][33], int cpip[20][2], int cp[100][12], int tp[100][102], bool &pack2, bool &pack3, bool &vitt)
{
    if(nessunpipotto(cpip)==true)
    {
        if(pack2==true && pack3==true)
        {
            vitt=true;
        }
        if(pack2==true && pack3==false)
        {
            zeroalldabullets(ent, cp, tp);
            pipottipack3(ent, cpip);
            pack3=true;
        }
        if(pack2==false)
        {
            zeroalldabullets(ent, cp, tp);
            pipottipack2(ent, cpip);
            pack2=true;
        }
    }
}

//main
int main()
{
    bool pack2=false;
    bool pack3=false;
    bool vitt=false;
    //inizializzazioni varie
    npipotti=0;
    vita=23;
    inizent(entity);
    inizcoordpipotti(coordpipotti);
    iniztraiettoria(traiettoriaproiettili);

    pipottipack1(entity, coordpipotti);

    //gioco
    system("pause");
    system("cls");
    do
    {
        gestiscipack(entity, coordpipotti, coordproiettili, traiettoriaproiettili, pack2, pack3, vitt);
        if(vitt==false)
        {
            barravita(vita, pack3);
            campo(entity);
            mossa(entity, coordpipotti, coordproiettili, traiettoriaproiettili, 0, true);  // 0 PER ORA
            gestiscipipotti(entity, coordpipotti, coordproiettili, traiettoriaproiettili);
            cancalldabullets(entity, coordproiettili, traiettoriaproiettili);
            gestisciproiettili(entity, coordpipotti, coordproiettili, traiettoriaproiettili, 5, vita);
        }
        system("cls");
    }while(coordpipotti[0][0]!=-1 && vitt==false);

    if(vitt==false) sconfitta(entity);
    else vittoria(entity);
}


//////
//////    //
//////


  //
