#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define N 20 
#define DIMENSIONE_INIZIALE 10000
#define DIMENSIONE_RICETTARIO 5000

//STRUCT MAGAZZINO
typedef struct ingr_mini{ //contiene lotti
	char nome[N+1];
	int quantita;
	int scadenza;
	struct ingr_mini *succ;
}ingr_mini;

typedef struct ingr_sommo{ //capo dei lotti
	char nome[N+1];
	int scadenza_min;
	int qtot;
	struct ingr_sommo *prox;
	struct ingr_mini *testa;
	struct ingr_mini *coda;
}ingr_sommo;

typedef struct{
	ingr_sommo **sommo;
	int dimensione;
}hash_table_magazzino;

//STRUCT RICETTARIO
typedef struct ingrediente_ricettario{
	int quantita;
	struct ingrediente_ricettario *succ;
	struct ingr_sommo *sommo;
}ingrediente_ricettario;

typedef struct recipe{
	char nome[N+1];		
	int peso;
	struct ingrediente_ricettario *testa;
	struct recipe *prox;
}recipe;

typedef struct{
	recipe **ricette;
	int dimensione;
}hash_table_ricettario;

//STRUCT CODE
typedef struct elemento{
	int quantita;
	int arrivo;
	int peso;
	struct recipe *ricetta;
	struct elemento *prox;
}elemento;

typedef struct coda{
	elemento *inizio;
	elemento *fine;
}coda;

// FUNZIONI AUSILIARIE - servono ovunque
void leggo_parola (char *buffer) //LEGGE UNA PAROLA SINGOLA E LA SALVA NEL BUFFER
{
	char car = getc(stdin); //carattere
	int i = 0;
	
	while (car != EOF && (car == '\n' || car == ' '))
		car = getc(stdin);
	
	if (car == EOF) //se finisco il file esco
        return;
		
	while(car != ' ' && car != '\n' && car != EOF)
	{
		buffer[i] = car;
		i++;
		car = getc(stdin);
	}
	buffer[i] = '\0';
	
	if (car == '\n')
        ungetc(car, stdin);
}

int controlla () //CONTROLLA SE HO FINITO LA RIGA - RITORNA 1 SE SI, 0 SE NO
{
	char car = getc(stdin); //carattere
	if (car == '\n')
		return 1;
	else
	{
		ungetc (car, stdin); //rimetto a posto ciò che ho letto
		return 0;
	}
}

int char_int (char *str) //PRENDE UN CHAR E LO TRASFORMA IN INT
{
	int int_finale = 0;
	
	for(int i = 0; str[i] != '\0'; i++)
		int_finale = int_finale * 10 + (str[i] - '0');

	return int_finale;
}

int trova_hash (char *nome, int dimensione) //RESTITUISCE IL VALORE DELLA FUNZIONE DI HASH DATO IL NOME DI UNA RICETTA/INGREDIENTE E LA DIMENSIONE DELLA HASH_TABLE
{
	unsigned long risultatohash = 5381;  // Valore iniziale dell'hash
    int c;

    while ((c = *nome++)) {
        risultatohash = ((risultatohash << 5) + risultatohash) + c;
    }
	risultatohash = risultatohash % dimensione;

    return (int)risultatohash;
}

//INIZIALIZZAZIONE FUNZIONI MAIN
hash_table_ricettario *crea_ricettario () //CREA UNA HASH TABLE PER IL RICETTARIO DI DIMENSIONE STANDARD
{
	hash_table_ricettario *ricettario = (hash_table_ricettario*) malloc(sizeof(hash_table_ricettario));
	ricettario->dimensione = DIMENSIONE_RICETTARIO;
	ricettario->ricette = (recipe **) malloc(ricettario->dimensione * sizeof(recipe *));
	
	for (int i = 0; i < ricettario->dimensione; i++)
		ricettario->ricette[i] = NULL;
		
	return ricettario;
}

hash_table_magazzino *crea_magazzino () //CREA UNA HASH TABLE PER IL MAGAZZINO DI DIMENSIONE STANDARD
{
	hash_table_magazzino *magazzino = (hash_table_magazzino*) malloc(sizeof(hash_table_magazzino));
	magazzino->dimensione = DIMENSIONE_INIZIALE;
	magazzino->sommo = (ingr_sommo **) malloc(magazzino->dimensione * sizeof(ingr_sommo *));
	
	for (int i=0; i < magazzino->dimensione; i++)
		magazzino->sommo[i] = NULL;
		
	return magazzino;
}

//MAGAZZINO - HASH TABLE
ingr_mini *crea_ingr_mini (char *buffer) //PRENDE LA STRINGA E LA LEGGE 3 VOLTE PER FARE UN INGREDIENTE CON NOME, QUANTITA E SCADENZA
{
	ingr_mini *ingr = (ingr_mini *) malloc(sizeof(ingr_mini));
	int i;
	
	leggo_parola(buffer); //leggo nome
	for (i = 0; buffer[i] != '\0'; i++)
		ingr->nome[i] = buffer[i]; 
	ingr->nome[i] = '\0';
	
	leggo_parola(buffer); //leggo quantita
	ingr->quantita = char_int(buffer);
    
	leggo_parola(buffer); //leggo scadenza
	ingr->scadenza = char_int(buffer);
	
	ingr->succ = NULL;
	return ingr;
}

ingr_sommo *crea_ingr_sommo (ingr_mini *ingr) //DATO UN INGR_MINI CREA IL SOMMO DI QUEL INGREDIENTE
{
	ingr_sommo *sommo = (ingr_sommo *) malloc(sizeof(ingr_sommo));
	
	sommo->qtot = ingr->quantita;
	sommo->scadenza_min = ingr->scadenza;
	sommo->prox = NULL;
	sommo->testa = sommo->coda = ingr;
	
	int i;
	for (i = 0; ingr->nome[i] != '\0'; i++) //do al sommo lo stesso nome del mini
		sommo->nome[i] = ingr->nome[i]; 
	sommo->nome[i] = '\0';
	
	return sommo;
}

hash_table_magazzino *inserisci_ingrediente (hash_table_magazzino *magazzino, ingr_mini *ingr) //INSERISCO L'INGREDIENTE NEL MAGAZZINO NELLA POSIZIONE CORRETTA
{
	int hash = trova_hash(ingr->nome, magazzino->dimensione);
	ingr_sommo *temp = magazzino->sommo[hash];

	if(temp == NULL) //non ho quel ingrediente nel magazzino - creo un sommo e lo inserisco nei suoi campi
	{
		temp = crea_ingr_sommo(ingr);
		magazzino->sommo[hash] = temp;
	}
	else //ho già qualcosa in quel hash - devo verificare cosa
	{
		ingr_sommo *precedente = NULL;
		
		//controllo se ho già lo stesso ingrediente o un altro con lo stesso hash
		while (temp != NULL)
		{
			//caso in cui il sommo in quel hash è quello del mio ingrediente:
			if(strcmp(temp->nome, ingr->nome) == 0)
			{
				temp->qtot += ingr->quantita; //aggiorno quantita sommo
				
				if(ingr->scadenza < temp->scadenza_min || temp->scadenza_min == -1)
					temp->scadenza_min = ingr->scadenza; //aggiorno prossima scadenza sommo
				
				if(temp->coda == NULL || temp->coda->scadenza < ingr->scadenza) //scade per ultimo rispetto agli altri lotti
				{
					if(temp->coda != NULL)
						temp->coda->succ = ingr;
					temp->coda = ingr;
					
					if(temp->testa == NULL)
						temp->testa = ingr;
					ingr->succ = NULL;
				}
				else //cerco la posizione in cui inserirlo e lo inserisco
				{
					ingr_mini *helper = temp->testa;
					ingr_mini *prev = NULL;
					
					while (helper != NULL && helper->scadenza < ingr->scadenza)
					{
	                    prev = helper;
	                    helper = helper->succ;
	                }
	                
	                if(helper != NULL && helper->scadenza == ingr->scadenza)
					{
						helper->quantita += ingr->quantita;
					}
					else
					{
						if (prev == NULL) //rimane come l'ho inizializzato - caso in cui la lista è vuota, perchè helper = helper->succ = NULL
						{
		                    ingr->succ = temp->testa;
		                    temp->testa = ingr;
		                }
						else //la lista non è vuota
						{
		                    prev->succ = ingr;
		                    ingr->succ = helper;
		                }
		                
		                if (ingr->succ == NULL) //sistemo la coda
    						temp->coda = ingr; 
					}
				}
				return magazzino;
			}
			else //non è il sommo corretto, scorro la lista dei sommi per vedere se ho quello del mio ingrediente
			{
				precedente = temp;
				temp = temp->prox;
			}
		}
		
		//se sono qui non ho il sommo del mio mini, perciò lo creo e lo inserisco in oridne agli altri
		temp = crea_ingr_sommo(ingr);
		precedente->prox = temp;
	}
	return magazzino;
}

hash_table_magazzino *rifornimento (char *buffer, hash_table_magazzino *magazzino, int t) //RESTITUISCE IL NODO CON SCADENZA MINIMA, DOPO AVER INSERITO LA STRINGA DI INPUT NEL MAGAZZINO ORDINATA IN BASE ALLA SCADENZA
{
	while (controlla() == 0) //scorro fino a fine riga
	{
		ingr_mini *ingr = crea_ingr_mini(buffer);
		if(t <= ingr->scadenza) //non inserisco lotti scaduti
		{
			magazzino = inserisci_ingrediente(magazzino, ingr);
		}
	}
	
	return magazzino;
}

// RICETTARIO - HASH TABLE
ingrediente_ricettario *crea_ingrediente (char *buffer, hash_table_magazzino *magazzino) //PRENDE LA STRINGA E LA LEGGE 2 VOLTE PER FARE UN INGREDIENTE CON NOME, QUANTITA E PUNTATORE AL SOMMO
{
	ingrediente_ricettario *ingr = (ingrediente_ricettario *) malloc(sizeof(ingrediente_ricettario));
	int i;
	
	leggo_parola(buffer); //leggo nome
	//capisco se ho quel ingr nel magazzino
	int hash = trova_hash(buffer, magazzino->dimensione);
	ingr_sommo *helper = magazzino->sommo[hash];
	
	if(helper == NULL) //non ho quel ingrediente nel magazzino - creo il sommo corrispondente
	{
		helper = (ingr_sommo *) malloc(sizeof(ingr_sommo));
		
		for (i = 0; buffer[i] != '\0' && i < N; i++)
			helper->nome[i] = buffer[i];
		helper->nome[i] = '\0';
		
		helper->qtot = 0;
		helper->scadenza_min = -1;
		helper->coda = helper->testa = NULL;
		helper->prox = NULL;
		magazzino->sommo[hash] = helper;
		ingr->sommo = helper; //sommo corrispondente
	}
	else //ho già qualcosa in quel hash - devo verificare cosa
	{
		ingr_sommo *precedente = NULL;
		ingr->sommo = NULL;
		
		//controllo se ho già lo stesso ingrediente o un altro con lo stesso hash
		while (helper != NULL)
		{
			//caso in cui il sommo in quel hash è quello del mio ingrediente:
			if(strcmp(helper->nome, buffer) == 0)
			{
				for (i = 0; buffer[i] != '\0' && i < N; i++) //salvo il nome al mio ingrediente
					helper->nome[i] = buffer[i];
				helper->nome[i] = '\0';
				
				ingr->sommo = helper;
				break;
			}
			else //non è il sommo corretto, scorro la lista dei sommi per vedere se ho quello del mio ingrediente
			{
				precedente = helper;
				helper = helper->prox;
			}
		}
		
		if(ingr->sommo == NULL) //non l'ho trovato nel ciclo di prima - lo creo e lo punto
		{
			helper = (ingr_sommo *) malloc(sizeof(ingr_sommo));
			
			for (i = 0; buffer[i] != '\0' && i < N; i++)
				helper->nome[i] = buffer[i];
			helper->nome[i] = '\0';
			
			helper->qtot = 0;
			helper->scadenza_min = -1;
			helper->prox = NULL;
			helper->coda = helper->testa = NULL;
			
			precedente->prox = helper;
			ingr->sommo = helper; //sommo corrispondente
		}
		
		free(precedente);
	}
	
	leggo_parola(buffer); //leggo quantita
	ingr->quantita = char_int(buffer);
	
	ingr->succ = NULL;
	
	return ingr;
}

recipe *crea_ricetta (char *buffer, hash_table_magazzino *magazzino) //CREA LA RICETTA CON IL NOME, GLI INGREDIENTI E IL PESO
{
	recipe *ricetta = (recipe *) malloc(sizeof(recipe));
	int i = 0;
	
	leggo_parola(buffer); //leggo nome
	for (i=0; buffer[i] != '\0' && i < N; i++)
		ricetta->nome[i] = buffer[i];
	ricetta->nome[i] = '\0';
	
	ricetta->prox = NULL;
	
	ingrediente_ricettario *ingr = crea_ingrediente(buffer, magazzino);
	ricetta->testa = ingr; //primo ingrediente della ricetta
	ricetta->peso = ingr->quantita;
	
	while (controlla() == 0) //scorro fino a fine riga
	{	
		ingrediente_ricettario *helper = crea_ingrediente(buffer, magazzino);
			
		ricetta->peso += helper->quantita;
		ingr->succ = helper;
		ingr = helper;
	}
	
	return ricetta;
}

hash_table_ricettario *inserisci_ricetta (hash_table_ricettario *ricettario, recipe *ricetta, int *check) //INSERISCE LA RICETTA NELLA HASH TABLE OPPURE IGNORA IL COMANDO
{
	int hash = trova_hash(ricetta->nome, ricettario->dimensione);
	recipe *temp = ricettario->ricette[hash];
	
	if(temp == NULL) //non ho la ricetta nel ricettario
	{
		ricettario->ricette[hash] = ricetta;
		*check = 1;
	}
	else //ho già qualcosa in quel hash - devo verificare cosa
	{
		recipe *precedente = NULL;
		
		//controllo se ho già la stessa ricetta o un'altra con lo stesso hash
		while (temp != NULL)
		{
			//caso in cui ho la stessa ricetta
			if(strcmp(temp->nome, ricetta->nome) == 0)
			{
				*check = 0; //ignoro il comando
				return ricettario;
			}
			else //non è la ricetta corretta, scorro la lista delle ricette per vedere se ce l'ho già
			{
				precedente = temp;
				temp = temp->prox;
			}
		}
		
		//se sono qui non ho la ricetta tra la lista di ricette presenti in quel hash
		precedente->prox = ricetta;
		*check = 1;
	}
	
	return ricettario;
}

int aggiungi_ricetta (char *buffer, hash_table_ricettario *ricettario, hash_table_magazzino *magazzino) //RESTITUISCE 0 PER "IGNORATO" O 1 PER "AGGIUNTA"
{
	recipe *ricetta = crea_ricetta(buffer, magazzino);

	int check;
	ricettario = inserisci_ricetta(ricettario, ricetta, &check);
	
	if (check == 1)	//non è presente nel ricettario - aggiunta - RETURN 1
	{
		return 1;		
	}
	else //è presente - ignorato - RETURN 0
	{
		free(ricetta);
		return 0;
	}
}

int rimuovi_ricetta (char *buffer, hash_table_ricettario *ricettario, coda *attesa, coda *evasi) //RESTITUISCE -1 PER "NON PRESENTE", 1 PER "ORDINI IN SOSPESO", 0 PER "RIMOSSA"
{
	leggo_parola(buffer); //è il nome della ricetta
	
	int hash = trova_hash(buffer, ricettario->dimensione);
	recipe *ricetta = ricettario->ricette[hash];
	
	if(ricetta == NULL) //non ho la ricetta nel ricettario
		return -1;
	else //ho già qualcosa in quel hash - devo verificare cosa
	{
		//verifico se nella lista delle ricette c'è la mia ricetta
		recipe *precedente = NULL;
		
		while (ricetta != NULL)
		{
			//caso in cui non ho la stessa ricetta
			if(strcmp(ricetta->nome, buffer) != 0)
			{
				precedente = ricetta;
				ricetta = ricetta->prox;
			}
			else //è la ricetta da eliminare 
			{
				//prima di poterla eliminare devo verificare se ci sono ordini in sospeso con quella ricetta - sia in attesa che in evasi
				if (attesa->inizio != NULL) 
				{
					elemento *helper = attesa->inizio; //per scorrere la lista
					while (helper != NULL)
					{
						int x = strcmp(helper->ricetta->nome, buffer); //controllo se il nome della ricetta coincide con quello dell'ordine
						if (x == 0)
							return 1;
						else
							helper = helper->prox;
					}
				}
				
				if (evasi->inizio != NULL) 
				{
					elemento *helper = evasi->inizio; //per scorrere la lista
					while (helper != NULL)
					{
						int x = strcmp(helper->ricetta->nome, buffer); //controllo se il nome della ricetta coincide con quello dell'ordine
						if (x == 0)
							return 1;
						else
							helper = helper->prox;
					}
				}
		
				//se sono qui non ci sono ordini in attesa o in evasi con quella ricetta: posso eliminarla dal ricettario
				if (precedente == NULL) //era il primo elem
					ricettario->ricette[hash] = ricetta->prox;
				else
					precedente->prox = ricetta->prox;

				free(ricetta);
				return 0; //avviso che l'ho rimossa
			}
		}
		
		//se sono qui non ho trovato la ricetta nella lista delle ricette
		return -1;
	}
}

//ORDINI - 2 CODE: 1 PER GLI ORDINI EVASI E 1 PER QUELLI IN ATTESA
recipe *trova_ricetta_ordine (hash_table_ricettario *ricettario, char *nome) //RESTITUISCE NULL SE NON E NEL RICETTARIO, ALTRIMENTI RESTITUISCE LA RICETTA STESSA
{
	int hash = trova_hash(nome, ricettario->dimensione);
	recipe *ricetta = ricettario->ricette[hash]; //è la ricetta che c'è in quel hash
	
	if(ricetta == NULL) //non ho la ricetta nel ricettario
		return NULL;
	else //ho già qualcosa in quel hash - devo verificare cosa
	{
		//verifico se nella lista delle ricette c'è la mia ricetta
		while (ricetta != NULL)
		{
			//caso in cui non ho la stessa ricetta
			if(strcmp(ricetta->nome, nome) != 0)
				ricetta = ricetta->prox;
			else //è la ricetta da restituire
				return ricetta;
		}
	}
	
	//dopo aver controllato la lista delle ricette vedo che non c'è la mia
	return NULL;
}

elemento *crea_ordine (char *buffer, int t, hash_table_ricettario *ricettario) //LEGGE LA STRINGA 2 VOLTE E RESTITUISCE L'ORDINE, INSERENDO ANCHE IL TEMPO A CUI E ARRIVATO
{
	elemento *ordine = (elemento *) malloc(sizeof(elemento));
	leggo_parola(buffer); //leggo il nome della ricetta
	recipe *ricetta = trova_ricetta_ordine(ricettario, buffer);
	
	if(ricetta == NULL) //non ho la ricetta
	{
		leggo_parola(buffer); //smaltisco la quantita per non buggare il resto
		return NULL;
	}
	else
	{
		ordine->ricetta = ricetta;
		ordine->arrivo = t;
		
		leggo_parola(buffer); //leggo quantità
		ordine->quantita = char_int(buffer);
	}
	
	return ordine;
}

ingr_mini *elimina_ingr(ingr_sommo *sommo, ingr_mini *da_eliminare) //RESTITUISCE IL NUOVO INGR_MINI TESTA DOPO AVER ELIMINATO LA TESTA E AGGIORNA IL SOMMO
{
	//printf("\nperchè cazzo sono in questa funzione se ho abbastanza quantita");
	ingr_mini *helper = da_eliminare->succ;
	
	if(helper == NULL) //ho finito la lista - lascio sommo per rifornimenti futuri per evitare di ricrearlo ogni volta
	{
		sommo->testa = sommo->coda = NULL;
		sommo->qtot = 0;
		sommo->scadenza_min = -1;
	}
	else //elimino il nodo interessato
	{
		sommo->testa = helper;
		sommo->qtot -= da_eliminare->quantita;
		sommo->scadenza_min = helper->scadenza;
		
		if (sommo->testa->succ == NULL)
        sommo->coda = sommo->testa;
	}
	
	free(da_eliminare);
	return sommo->testa;
}

int verifica_disponibilita(ingrediente_ricettario *ingr, int qnecessaria, int scadenza) //RESTITUISCE 0 SE NON HO L'INGREDIENTE O NON NE HO ABBASTANZA, 1 SE SI
{
	//verifico se ci sono lotti scaduti
	while(ingr->sommo->testa != NULL && scadenza >= ingr->sommo->testa->scadenza) //ho lotti di ingr scaduti
		ingr->sommo->testa = elimina_ingr(ingr->sommo, ingr->sommo->testa); //mi restituisce la nuova testa e mi aggiorna le info di sommo
			
	if(qnecessaria <= ingr->sommo->qtot) //ho eliminato i lotti scaduti e ho quantita sufficiente
		return 1;
	else
		return 0;	
}

ingr_sommo *da_modificare(hash_table_magazzino *magazzino, char *nome, int qnecessaria) //RESTITUISCE L'INGR_SOMMO A CUI DOVRO MODIFICARE LA LISTA
{
	int hash = trova_hash(nome, magazzino->dimensione);
	
	ingr_sommo *temp = magazzino->sommo[hash];
	
	while (temp != NULL)
	{
		//verifico se il sommo in quel hash è quello del mio ingrediente
		if(strcmp(temp->nome, nome) == 0) //lo è
			return temp;				
		else //non lo è
			temp = temp->prox;
	}
	return NULL;
}

int ordine (char *buffer, int t, coda *evasi, coda *attesa, hash_table_ricettario *ricettario, hash_table_magazzino *magazzino) //RESTITUISCE 1 PER "ACCETTATO" E 0 PER "RIFIUTATO"
{
	elemento *ordine = crea_ordine(buffer, t, ricettario); //creo l'ordine
	
	if (ordine == NULL) //non è presente nel ricettario
		return 0;
	else //è presente, devo verificare se posso farla, se sì l'aggiungo in coda evasi, sennò la metto in coda attesa
	{
		ordine->peso = ordine->ricetta->peso * ordine->quantita; //per inserirle nel camioncino
		int check = 1;
		
		ingrediente_ricettario *ring = ordine->ricetta->testa; //è il primo ingrediente
		
		while(ring != NULL) //verifico se posso fare la ricetta
		{
			int qnecessaria = ring->quantita * ordine->quantita;
			check = verifica_disponibilita(ring, qnecessaria, t);
			
			if (check == 0) //ingrediente non presente o in quantita insufficiente - aggiungo in attesa
			{
				if (attesa->inizio == NULL)
					attesa->inizio = ordine;
				else
					attesa->fine->prox = ordine;
				
				attesa->fine = ordine;
				ordine->prox = NULL;
				
				return 1;
			}
			
			ring = ring->succ; //scorro
		}
		
		//se sono qui posso fare la ricetta:
		//inserisco l'ordine nella coda degli ordini evasi
		if (evasi->inizio == NULL)
			evasi->inizio = ordine;
		else
			evasi->fine->prox = ordine;
		evasi->fine = ordine;
		ordine->prox = NULL;
		
		//diminuisco le quantita degli ingredienti nel magazzino in base a quanto ne uso per la ricetta - ok
		ring = ordine->ricetta->testa; 
		while(ring != NULL) //scorro gli ingredienti della ricetta nel magazzino e diminuisco la loro quantita
		{
			int qnecessaria = ring->quantita * ordine->quantita;
			ingr_sommo *sommo = da_modificare(magazzino, ring->sommo->nome, qnecessaria); //ho il sommo di quel ingrediente che ora devo modificare
			ingr_mini *helper = sommo->testa;
			
			while(qnecessaria > 0 && helper != NULL)
			{
				if(helper->scadenza > t)
				{
					if(helper->quantita > qnecessaria) //ho più g di quel ingrediente rispetto a quello che mi serve nel primo ingr_mini
					{
						helper->quantita -= qnecessaria;
						sommo->qtot -= qnecessaria;
						qnecessaria = 0;
					}
					else if (helper->quantita == qnecessaria) //ho gli stessi g
					{
						qnecessaria = 0;
						helper = elimina_ingr(sommo, helper);
					}
					else //ho meno g nell'ingrediente dei necessari, perciò devo eliminarlo e passare al prossimo
					{
						qnecessaria -= helper->quantita;
						helper = elimina_ingr(sommo, helper);
					}
				}
				else
					helper = helper->succ;
			}
			
			ring = ring->succ; //scorro
		}
	}
	return 1;
}

void controlla_ordini (coda *evasi, coda *attesa, hash_table_ricettario *ricettario, hash_table_magazzino *magazzino, int t) //CONTROLLA LA CODA ATTESA E FA GLI ORDINI DI CUI HO GLI INGREDIENTI
{
	elemento *ordine = attesa->inizio; //con questo scorro la lista attesa
	elemento *precedente = NULL;

	while(ordine != NULL)
	{
		elemento *ordine_succ = ordine->prox;
		int check = 1;
		
		ingrediente_ricettario *ring = ordine->ricetta->testa; //è il primo ingrediente - mi serve per scorrere gli ingredienti
		while(ring != NULL) //verifico se posso fare la ricetta
		{
			int qnecessaria = ring->quantita * ordine->quantita;
			if(ring->sommo->qtot < qnecessaria) //non ne ho abbastanza (non controllo nemmeno se tra quella che ho ci sono lotti scaduti)
				check = 0;
			else
				check = verifica_disponibilita(ring, qnecessaria, t);
				
			if (check == 0) //ingrediente non presente o in quantita insufficiente
				break;
			
			ring = ring->succ; //scorro
		}
		
		if(check == 0) //vado al prossimo ordine della lista
		{
			precedente = ordine;
			ordine = ordine_succ;
			continue;
		}
		
		//se sono qui posso fare la ricetta:
		//rimuovo l'ordine dalla lista attesa
		if(precedente == NULL) //ordine è il primo elemento della lista
		{
			attesa->inizio = ordine->prox;
			if(attesa->inizio == NULL)
				attesa->fine = NULL;
		}
		else //ordine non è il primo elemento della lista attesa
		{
			precedente->prox = ordine->prox;
			if (ordine->prox == NULL)
				attesa->fine = precedente;
		}
		ordine->prox = NULL;
		
		//aggiungo l'ordine nella lista degli ordini completati nella posizione giusta in base al tempo d'arrivo
		if (evasi->inizio == NULL) //primo della lista
		{
			evasi->inizio = ordine;
			evasi->fine = ordine;
		}
		else
		{
			elemento *helper = evasi->inizio;
			elemento *precedente_evasi = NULL;
			
			while (helper != NULL && ordine->arrivo > helper->arrivo) //trovo la posizione
			{
				precedente_evasi = helper;
				helper = helper->prox;
			}
			
			if(precedente_evasi == NULL) //se rimane come l'ho inizializzato -> helper è il primo
			{
				ordine->prox = evasi->inizio;
				evasi->inizio = ordine;
			}
			else //helper non è il primo della lista
			{
				precedente_evasi->prox = ordine;
				ordine->prox = helper;
				if (helper == NULL) //in caso sia l'ultimo
					evasi->fine = ordine;
			}
		}
		
		//diminuisco le quantita degli ingredienti nel magazzino in base a quanto ne uso per la ricetta - ok
		ring = ordine->ricetta->testa;
		while(ring != NULL) //scorro gli ingredienti della ricetta nel magazzino e diminuisco la loro quantita
		{
			int qnecessaria = ring->quantita * ordine->quantita;
			ingr_sommo *sommo = da_modificare(magazzino, ring->sommo->nome, qnecessaria); //ho il sommo di quel ingrediente che ora devo modificare
			ingr_mini *helper = sommo->testa;
			
			while(qnecessaria > 0 && helper != NULL)
			{
				if(helper->scadenza > t)
				{
					if(helper->quantita > qnecessaria) //ho più g di quel ingrediente rispetto a quello che mi serve nel primo ingr_mini
					{
						helper->quantita -= qnecessaria;
						sommo->qtot -= qnecessaria;
						qnecessaria = 0;
					}
					else if (helper->quantita == qnecessaria) //ho gli stessi g
					{
						qnecessaria = 0;
						helper = elimina_ingr(sommo, helper);
					}
					else //ho meno g nell'ingrediente dei necessari, perciò devo eliminarlo e passare al prossimo
					{
						qnecessaria -= helper->quantita;
						helper = elimina_ingr(sommo, helper);
					}
				}
				else
					helper = helper->succ;
			}
			
			ring = ring->succ; //scorro
		}

		ordine = ordine_succ;
	}
}

//FUNZIONI PER IL MAIN
void ordina_decrescente (elemento **array, int max) //ORDINA L'ARRAY IN ORDINE DECRESCENTE DI PESO E TIENE CONTO DELL'ORDINE D'ARRIVO
{
	for (int i = 0; i < max; i++)
	{
		for (int j = i + 1; j < max; j++) //non ricontrollo gli elementi che ho già ordinato
		{
			if(array[i]->peso < array[j]->peso)
			{
				elemento *help = array[i]; //per salvare l'elemento attuale e fare lo swap
				array[i] = array[j];
				array[j] = help;
			}
			else if (array[i]->peso == array[j]->peso)
			{
				if(array[i]->arrivo > array[j]->arrivo) //si scambiano di posto in base alla posizione di arrivo
				{
					elemento *help = array[i];
					array[i] = array[j];
					array[j] = help;
				}
			}
		}
	}
}

void gestione_evasi (coda *evasi, int capienza) //SI OCCUPA DI TOGLIERE DA EVASI GLI ELEMENTI CHE POSSONO ESSERE SPEDITI E GLI STAMPA IN ORDINE DECRESCENTE DI PESO
{
	elemento *helper = evasi->inizio; //scorro la coda evasi
	elemento *prev = NULL;
	int p = 0, max = 0, dimensione = 100;
	elemento **vett = (elemento **) malloc(dimensione * sizeof(elemento *));
	
	while (helper != NULL && p + helper->peso <= capienza)
	{
		if (max >= dimensione) //vedo che posso spedire più elementi di quanti spazi ho nell'array
		{
			dimensione = dimensione * 2;
			vett = (elemento **) realloc(vett, dimensione * sizeof(elemento *));
		}
		
		vett[max] = helper;
		p += helper->peso; //modifico il peso così nel while verifico se aggiungendo anche il prossimo ordine sto sotto la capienza max
		
		//elimino l'ordine dalla coda evasi
		if (prev == NULL) //primo elemento
			evasi->inizio = helper->prox;
		else
			prev->prox = helper->prox;
		
		if (evasi->fine == helper) //ultimo elemeto
			evasi->fine = prev;
		helper = helper->prox;
		
		max++;
	}
	
	if (evasi->inizio == NULL) //se svuoto la lista evasi sistemo la fine
        evasi->fine = NULL;

	ordina_decrescente(vett, max);
	
	for (int i = 0; i < max; i++)
		printf("%d %s %d\n", vett[i]->arrivo, vett[i]->ricetta->nome, vett[i]->quantita);
	
	free(vett);
}

int comando (char *buffer) //CAPISCO IL COMANDO E RESTITUISCO IL CODICE DEL COMANDO PER POI ESEGUIRLO NEL MAIN
{
    char str1[]= "aggiungi_ricetta", str2[]= "rimuovi_ricetta", str3[]= "rifornimento", str4[]= "ordine";
	int c;
	
	c = strcmp(buffer, str1);
	if(c == 0) //AGGIUNGI RICETTA
	    return 1;	
	else
	{
		c = strcmp(buffer, str2);
	    if(c == 0) //RIMUOVI RICETTA
	    	return 2;
	    else
	    {
			c = strcmp(buffer, str3);
		    if(c == 0) //RIFORNIMENTO
		    	return 3;
		    else
		    {
				c = strcmp(buffer, str4);
			    if(c == 0) //ORDINE
			    	return 4;
			    else
			    	return c;
			}
		}
	}
}

int main()
{
	int t = 0, periodo, capienza;
	char buffer[N+1], c;
	
	//INIZIALIZZAZIONE DELLE STRUTTURE NECESSARIE PER RICETTARIO, MAGAZZINO E ORDINI
	hash_table_ricettario *ricettario = crea_ricettario(DIMENSIONE_INIZIALE); //HASH TABLE VUOTA PER RICETTARIO
	hash_table_magazzino *magazzino = crea_magazzino(); //HASH TABLE VUOTA PER MAGAZZINO
	coda *evasi = (coda *) malloc(sizeof(coda)); //CODA VUOTA PER GLI ORDINI CHE SI POSSONO FARE
		evasi->inizio = evasi->fine = NULL;
	coda *attesa = (coda *) malloc(sizeof(coda)); //CODA VUOTA PER GLI ORDINI IN ATTESA DI INGREDIENTI
		attesa->inizio = attesa->fine = NULL;
	
	//CONFIGURAZIONE CORRIERE - ok
	//printf("t: -");
	leggo_parola(buffer);
	periodo = char_int(buffer);
	leggo_parola(buffer);
	capienza = char_int(buffer);

	while ((c = getc(stdin)) != EOF)
	{
		ungetc(c, stdin);
		
		//PASSAGGIO DEL CORRIERE
		if (t > 0) //escludo t = 0 dal conteggio
		{
			if(t % periodo == 0) //l'istante t è un multiplo del periodo 
			{
				if (evasi->inizio == NULL)
					printf("camioncino vuoto\n");
				else
					gestione_evasi(evasi, capienza);
			}
		} 	
		
		leggo_parola(buffer);
		int check = comando(buffer);
		if (check == 1) //AGGIUNGI RICETTA -> 0 PER "IGNORATO" OPPURE 1 PER "AGGIUNTA"
		{
			int x = aggiungi_ricetta(buffer, ricettario, magazzino);
			if (x == 1)
				printf("aggiunta\n");
			else if (x == 0)
				printf("ignorato\n");
		}
		else if (check == 2) //RIMUOVI RICETTA -> -1 PER "NON PRESENTE", 1 PER "ORDINI IN SOSPESO" OPPURE 0 PER "RIMOSSA"
		{
			int x = rimuovi_ricetta(buffer, ricettario, attesa, evasi);
			if (x == -1)
				printf("non presente\n");
			else if (x == 1)
				printf("ordini in sospeso\n");
			else if (x == 0)
				printf("rimossa\n");
		}
		else if (check == 3) //RIFORNIMENTO
		{
			magazzino = rifornimento(buffer, magazzino, t);
			controlla_ordini(evasi, attesa, ricettario, magazzino, t);
			printf("rifornito\n");
		}
		else if (check == 4) //ORDINE -> 1 PER "ACCETTATO" OPPURE 0 PER "RIFIUTATO"
		{
			int x = ordine(buffer, t, evasi, attesa, ricettario, magazzino);
			if (x == 1)
				printf("accettato\n");
			else if (x == 0)
				printf("rifiutato\n");
		}
		
		t++;
	}
	
	//PASSAGGIO FINALE DEL CORRIERE
	if(t % periodo == 0) //l'istante t è un multiplo del periodo 
		if (evasi->inizio != NULL)
			gestione_evasi(evasi, capienza);
	
	free(ricettario);
	free(magazzino);
    free(evasi);
    free(attesa);
	
	return 0;
}
