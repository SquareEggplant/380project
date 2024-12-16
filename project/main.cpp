#include <iostream>
#include <cstdint>
#include <SDL3/SDL.h>
#include <string>

//Global Variables (You can laugh if you want)
const int width = 1200, height = 800;
int Global_En_Passant_Flag = -1, wpoints=0, bpoints=0;
bool visionstate = 0;
SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture *board, *panel, *redshade, *greenshade, *pieces[13], *digits[10];

class tile
{
    public:
    SDL_FRect tileFRect;
    
    //Empty = 0, King = 1, Pawn = 2, Knight = 3, Bishop = 4, Rook = 5, Queen = 6
    int piece; //Black is negative, White is positive

    bool canMove[64]; //Tiles that said peice can move to
    bool controlState[3]; //000 = Uncontested, 100 = White Control, 001 = Black Control, 101 = Contested
    bool specialFlag = 0; //Handles Castling
    int farmstate = 0;
};

//Create Array of tiles
tile tileArr[64];

//Me when I didn't put any of these in a class
bool init();
void close();
bool loadBoard();
bool renderall(Uint8 tile = 255);
SDL_Texture* loadTexture( std::string path );

bool validMove(int tile1, int tile2, int whosturn); //Why is this separate from movePiece?
int movePiece(int tile1, int tile2); //Returns the captured piece
void findMoves(tile* tileArr); //Takes the entire board and finds the valid moves for each peice
void MovableControl(int tileon, int tileto); //Helper function for findMoves, edits the control state of tiles and adds allowed moves
Uint8 getTileOn(int x, int y); //turns x and y coordinates into board tiles
int kingcheck();

int main(int argc, char *argv[]){
    //Start up SDL and create window
    if(init()) printf( "Failed to initialize!\n" );

    //Load the board
    if(loadBoard()) printf( "Failed to load board!\n" );
    
    //Load Renderer
    renderer = SDL_CreateRenderer(window, NULL);
    if( renderer == NULL ){
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
    }
    else{
            //Initialize renderer color
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        }
    
    //Load my really primitive shaders
    redshade = loadTexture("images/redshade.bmp");
    greenshade = loadTexture("images/greenshade.bmp");

    //Loads my numbers
    for(int i=0; i<10; i++) digits[i] = loadTexture("images/" + std::to_string(i) + ".bmp");
    
    //Load Pieces and Board
    pieces[7] = loadTexture("images/wKing.bmp");
    pieces[8] = loadTexture("images/wPawn.bmp");
    pieces[9] = loadTexture("images/wKnight.bmp");
    pieces[10] = loadTexture("images/wBishop.bmp");
    pieces[11] = loadTexture("images/wRook.bmp");
    pieces[12] = loadTexture("images/wQueen.bmp");
    //Entry 6 will be empty for math reasons
    pieces[5] = loadTexture("images/bKing.bmp");
    pieces[4] = loadTexture("images/bPawn.bmp");
    pieces[3] = loadTexture("images/bKnight.bmp");
    pieces[2] = loadTexture("images/bBishop.bmp");
    pieces[1] = loadTexture("images/bRook.bmp");
    pieces[0] = loadTexture("images/bQueen.bmp");
    board = loadTexture("images/chessboard.bmp");
    panel = loadTexture("images/panel.bmp");
    
    SDL_Event event;

    float xi,yi,xf,yf,temptx,tempty;
    int tilei, tilef;//The tile where our peice moves from
    bool readon = false, panelon = false, forceclose = false;
    int whosturn = 1;
    int winner = 0;
    int activebutton = 0;

    //Find the valid moves
    findMoves(tileArr);
    renderall();

    while(winner == 0 && !forceclose){
        if(SDL_PollEvent(&event)){
            if(event.type == SDL_EVENT_QUIT) forceclose = true;

            if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                SDL_GetMouseState(&xi,&yi);
                //Start reading mouse movements
                tilei = getTileOn(xi,yi);
                //read the x and y positions of tilei
                if(activebutton == 100){
                    if(tilei<64){
                        if(whosturn==1 && tileArr[tilei].farmstate==0 && tileArr[tilei].piece>0 && wpoints>=200){
                            tileArr[tilei].farmstate=1;
                            tileArr[tilei].controlState[0]=1;
                            wpoints-=200;
                        }
                        if(whosturn==-1 && tileArr[tilei].farmstate==0 && tileArr[tilei].piece<0 && bpoints>=200){
                            tileArr[tilei].farmstate=-1;
                            tileArr[tilei].controlState[2]=1;
                            bpoints-=200;
                        }
                    }
                    activebutton = 0;
                }

                if(activebutton == 101){
                    if(tilei<64){
                        if(whosturn==1 && tileArr[tilei].farmstate==1 && tileArr[tilei].piece>0){
                            tileArr[tilei].farmstate=0;
                            tileArr[tilei].controlState[0]=0;
                            wpoints+=100;
                        }
                        if(whosturn==-1 && tileArr[tilei].farmstate==-1 && tileArr[tilei].piece<0){
                            tileArr[tilei].farmstate=0;
                            tileArr[tilei].controlState[2]=0;
                            bpoints+=100;
                        }
                    }
                    activebutton = 0;
                }

                if(activebutton == 102){
                    if(tilei<64){
                        if(whosturn==1 && wpoints>=1000 && tileArr[tilei].controlState[2]==0){
                            tileArr[tilei].piece=0;
                            wpoints-=1000;
                            findMoves(tileArr);
                        }
                        if(whosturn==-1 && bpoints>=1000 && tileArr[tilei].controlState[0]==0){
                            tileArr[tilei].piece=0;
                            bpoints-=1000;
                            findMoves(tileArr);
                        }
                    }
                    activebutton = 0;
                }

                if(activebutton == 103){
                    if(tilei<64){
                        if(whosturn==1 && wpoints>=1000 && tileArr[tilei].controlState[2]==0 && tileArr[tilei].controlState[0]==1){
                            tileArr[tilei].piece=1;
                            wpoints-=1000;
                            findMoves(tileArr);
                        }
                        if(whosturn==-1 && bpoints>=1000 && tileArr[tilei].controlState[0]==0 && tileArr[tilei].controlState[2]==1){
                            tileArr[tilei].piece=-1;
                            bpoints-=1000;
                            findMoves(tileArr);
                        }
                    }
                    activebutton = 0;
                }
                
                else{
                    if (tilei<64){
                        readon = true;
                        temptx = tileArr[tilei].tileFRect.x;
                        tempty = tileArr[tilei].tileFRect.y;

                        //Snap to mouse
                        tileArr[tilei].tileFRect.x = xi-48;
                        tileArr[tilei].tileFRect.y = yi-48;
                    } 
                }

                if (tilei>=100) panelon = true;

                renderall();
            }

            if(readon == true){
                if(event.type == SDL_EVENT_MOUSE_MOTION){
                    SDL_GetMouseState(&xf,&yf);
                    tileArr[tilei].tileFRect.x = xf-48;
                    tileArr[tilei].tileFRect.y = yf-48;
                    renderall(tilei);
                }
            
                if(event.type == SDL_EVENT_MOUSE_BUTTON_UP){
                    SDL_GetMouseState(&xf,&yf);
                    tilef = getTileOn(xf,yf);
                    //Reset tile state
                    tileArr[tilei].tileFRect.x = temptx;
                    tileArr[tilei].tileFRect.y = tempty;

                    if (validMove(tilei,tilef,whosturn)) {
                        movePiece(tilei,tilef);
                        findMoves(tileArr);
                        whosturn*=-1;
                        if(whosturn == 1){
                            int countcontrol = 0;
                            for(int i=0; i<64; i++) if(tileArr[i].controlState[0]) countcontrol+=1;
                            wpoints += countcontrol;
                        }
                        else{
                            int countcontrol = 0;
                            for(int i=0; i<64; i++) if(tileArr[i].controlState[2]) countcontrol+=1;
                            bpoints += countcontrol;
                        }

                        /*
                        for(int i=0; i<8; i++) {
                            for(int j=0; j<8; j++) {
                            //std::cout<<tileArr[8*i+j].controlState[0];
                            //std::cout<<tileArr[tilef].canMove[i*8+j]<<" ";
                            }
                            std::cout<<std::endl;
                        }
                        */
                    }
                    winner = kingcheck();
                    renderall();
                    renderall();
                    readon = false;
                }
            }

            if(panelon == true){
                if(event.type == SDL_EVENT_MOUSE_BUTTON_UP){
                    SDL_GetMouseState(&xf,&yf);
                    int button = getTileOn(xf,yf);
                    if(button == 100) activebutton = 100;
                    if(button == 101) activebutton = 101;
                    if(button == 102) activebutton = 102; 
                    if(button == 103) activebutton = 103; 
                    if(button == 104) visionstate = !visionstate;
                    panelon == false;
                    renderall();
                }
            }
            
            if(event.type == SDL_EVENT_KEY_DOWN) {
                if(event.key.key == SDLK_X) {
                    renderall();
                }
                
                if(event.key.key == SDLK_Y) {
                SDL_RenderClear(renderer);
                SDL_RenderPresent(renderer);
                }   
            }
        }
    }

    std::string message[3] = {"Black wins!", "Nobody wins?", "White wins!"};

    std::cout<<message[1+winner];
    
    while(!forceclose){
        if(SDL_PollEvent(&event)){
            if(event.type == SDL_EVENT_QUIT) break;
            if(event.type == SDL_EVENT_KEY_DOWN) if(event.key.key == SDLK_X) break;
        }
    }

    close();
    
    return 0;
}

bool init(){
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow( "Really cool window", width, height, SDL_WINDOW_HIGH_PIXEL_DENSITY);
    
    if(window==NULL)
    {
        std::cout<< "Could not create window: " << SDL_GetError() << std::endl;
        return 1;
    }

    return 0;
}

void close(){
    //Not sure if I NEED to destroy everything but I guess it's best practice
    SDL_DestroyTexture(board);
    board = NULL;

    for(int i=0; i<13; i++){
    SDL_DestroyTexture(pieces[i]);
    pieces[i] = NULL;
    }

    SDL_DestroyRenderer(renderer);
    renderer = NULL;

    SDL_DestroyWindow(window);
    window = NULL;

    SDL_Quit();
}

SDL_Texture* loadTexture( std::string path ){
    //The final texture
    SDL_Texture* newTexture = NULL;

    //Load image at specified path
    SDL_Surface* tempSurf = SDL_LoadBMP(path.c_str());
    if( tempSurf == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), SDL_GetError() );
    }
    else
    {
        //Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface(renderer, tempSurf);
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }

        //Get rid of old loaded surface
        SDL_DestroySurface( tempSurf );
    }

    return newTexture;
}

bool renderall(Uint8 tile){
    SDL_RenderClear(renderer);
    SDL_FRect bfrect, pfrect, digit;

    //Board Render
    bfrect.x = 0, bfrect.y=0, bfrect.h=800, bfrect.w=800;
    SDL_RenderTexture(renderer, board, NULL, &bfrect);

    //Panel Render
    pfrect.x = 840, pfrect.y=160, pfrect.h=480, pfrect.w=320;
    SDL_RenderTexture(renderer, panel, NULL, &pfrect);

    //Render Points
    digit.x = 840, digit.y=664, digit.h=56, digit.w=28;
    int tempoints = wpoints;
    int power = 10000;
    int pdigit;
    for(int i=0; i<5; i++){
        pdigit = tempoints/power;
        SDL_RenderTexture(renderer, digits[pdigit], NULL, &digit);
        tempoints -= power*pdigit;
        power/=10;
        digit.x+=32;
    }

    digit.x = 840, digit.y=80;
    tempoints = bpoints;
    power = 10000;
    for(int i=0; i<5; i++){
        pdigit = tempoints/power;
        SDL_RenderTexture(renderer, digits[pdigit], NULL, &digit);
        tempoints -= power*pdigit;
        power/=10;
        digit.x+=32;
    }

    //Render pieces
    for(int i=0; i<64; i++){
        SDL_RenderTexture(renderer, pieces[6+tileArr[i].piece], NULL, &tileArr[i].tileFRect);
        if(tileArr[i].farmstate!=0){
            if(tileArr[i].controlState[0]==1) SDL_RenderTexture(renderer, greenshade, NULL, &tileArr[i].tileFRect);
            if(tileArr[i].controlState[2]==1) SDL_RenderTexture(renderer, redshade, NULL, &tileArr[i].tileFRect);
        }
    }

    //Render the piece we clicked above everything else
    if(tile<64) SDL_RenderTexture(renderer, pieces[6+tileArr[tile].piece], NULL, &tileArr[tile].tileFRect);
    
    //Control vision state
    if(visionstate){
        for(int i=0; i<64; i++){
            if(tileArr[i].controlState[0]==1) SDL_RenderTexture(renderer, greenshade, NULL, &tileArr[i].tileFRect);
            if(tileArr[i].controlState[2]==1) SDL_RenderTexture(renderer, redshade, NULL, &tileArr[i].tileFRect);
        }
    }

    SDL_RenderPresent(renderer);

    return 0;
}

bool loadBoard(){
    //Behold the board
    int StartingBoard[64]= {
    -5,-3,-4,-1,-6,-4,-3,-5,
    -2,-2,-2,-2,-2,-2,-2,-2,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0,
     2, 2, 2, 2, 2, 2, 2, 2,
     5, 3, 4, 6, 1, 4, 3, 5};

    //Tile(1,1) = (16.16), Tile size = 96px
    float xpos = 16;
    float ypos = 16;
    int i = 0;
    for(int row=0; row<8; row++){
        for(int col=0; col<8; col++){
            tileArr[i].tileFRect.x = xpos;
            tileArr[i].tileFRect.y = ypos;
            tileArr[i].tileFRect.w = 96;
            tileArr[i].tileFRect.h = 96;
            tileArr[i].piece = StartingBoard[i];
            i++;
            xpos+=96;
            //std::cout<<"("<<tileArr[i].tileFRect.w<<tileArr[i].tileFRect.h<<")"<<std::endl;
        }
        xpos=16;
        ypos+=96;
    }
    
    //Initialize Castling Flags
    int castlingflag[6] = {0,3,7,56,60,63};
    for(int i=0; i<6; i++) tileArr[castlingflag[i]].specialFlag = 1;
    
    return 0;
}

int movePiece(int tile1, int tile2){
    
    if((Uint8)tile2<64){
        int p = tileArr[tile1].piece;
        int sign = -1;
        if(p>0) sign = 1;
        //Clear Special Flags
        tileArr[tile1].specialFlag = 0;
        tileArr[tile2].specialFlag = 0;

        //Perform Capture
        int capture = tileArr[tile2].piece;
        tileArr[tile2].piece = p;
        tileArr[tile1].piece = 0;

        if(tileArr[tile2].farmstate*p<0) tileArr[tile2].farmstate = sign;

        //Castling
        if(abs(p) == 1) {
            if(tile2-tile1==2) {
                int trook = tile1+7-tile1%8;
                tileArr[trook].piece=0;
                tileArr[trook].specialFlag=0;
                tileArr[tile2-1].piece=5*p;
            }

            if(tile2-tile1==-2) {
                int trook = tile1-tile1%8;
                tileArr[trook].piece=0;
                tileArr[trook].specialFlag=0;
                tileArr[tile2+1].piece=5*p;
            }
        }

        //En Passant
        if(abs(p) == 2) {  
            if(capture == 0 && tile2+4*p == Global_En_Passant_Flag){ //Execute En Passant
                capture = tileArr[tile2+4*p].piece;
                tileArr[tile2+4*p].piece = 0;
            }
            
            if(tile1 == tile2+p*8) Global_En_Passant_Flag = tile2; //Allow En Passant
            else Global_En_Passant_Flag = -1;
        }
        else Global_En_Passant_Flag = -1;

        return capture;
    }

    else{
         std::cout << "Error at movePiece(), tile2 out of bounds"<<std::endl;
         return 100;
    }
}

bool validMove(int tile1, int tile2, int whosturn){
    if(tile1<-1 || tile1>64) return false;
    if(tile2<-1 || tile2>64) return false;
    if(tileArr[tile1].piece*whosturn < 0) return false; // Empty tile can't move

    if(tileArr[tile1].canMove[tile2] == 1) return true;
    else return false;
}

void findMoves(tile* tileArr){
    //Clear Board State
    for(int i=0; i<64; i++){
        tileArr[i].controlState[0] = 0;
        tileArr[i].controlState[2] = 0;
        tileArr[i].controlState[1-tileArr[i].farmstate] = 1;
        for(int j=0; j<64; j++) tileArr[i].canMove[j] = 0;
    }

    //Iterate for all tiles
    for(Uint8 tileon=0; tileon<64; tileon++){
        //Useful Values
        int p = tileArr[tileon].piece;
        while(p==0) {//Hopefully this skips a few comparisons
            tileon++;
            p = tileArr[tileon].piece;
        }

        bool s = tileArr[tileon].specialFlag;
        int sign = -1;
        if(p>0) sign = 1;

        //King Case (Control State Only)
        if(abs(p) == 1){
            int start = 0, end = 8;

            Uint8 kingspec[8] = {tileon-9, tileon-1, tileon+7, tileon-8, tileon+8, tileon-7, tileon+1, tileon+9};
            if(tileon%8==0) start = 3;
            if(tileon%8==7) end = 5;
            
            for(int i=start;i<end;i++){
                if(kingspec[i]<64){
                    tileArr[kingspec[i]].controlState[1-p] = 1;
                }
            }
        }

        //Pawn Case
        if(abs(p) == 2){
            Uint8 tileu = tileon-8;
            if(tileu<48){
                if(tileArr[tileon-p*4].piece==0){
                    //Pawns can move but they dont control
                    tileArr[tileon].canMove[tileon-p*4]=1;
                    //Also allow move 2 spaces if on 2nd/6th rank
                    if(tileon>=28+p*10 && tileon<36+p*10 && tileArr[tileon-p*8].piece==0) tileArr[tileon].canMove[tileon-p*8]=1;
                }
                
                if(tileon%8!=7) {
                    MovableControl(tileon, tileon-p*4+1);
                    if(p*tileArr[tileon-p*4+1].piece>=0) tileArr[tileon].canMove[tileon-p*4+1]=0;
                    if(p*tileArr[tileon-p*4+1].piece==0 && p*tileArr[tileon+1].piece==-4 && tileon+1==Global_En_Passant_Flag) tileArr[tileon].canMove[tileon-p*4+1]=1;
                }
                if(tileon%8!=0) {
                    MovableControl(tileon, tileon-p*4-1);
                    if(p*tileArr[tileon-p*4-1].piece>=0) tileArr[tileon].canMove[tileon-p*4-1]=0;
                    if(p*tileArr[tileon-p*4-1].piece==0 && p*tileArr[tileon-1].piece==-4 && tileon-1==Global_En_Passant_Flag) tileArr[tileon].canMove[tileon-p*4-1]=1;
                }
            }
        }

        //Knight Case
        if(abs(p) == 3){
            int start = 0, end = 8;
            Uint8 knightspec[8] = {tileon-10, tileon+6, tileon-17, tileon+15, tileon-15, tileon+17, tileon-6, tileon+10};
            if(tileon%8==0) start = 4;
            if(tileon%8==1) start = 2;
            if(tileon%8==6) end = 6;
            if(tileon%8==7) end = 4;

            for(int i=start;i<end;i++){
                //Uint8 a = knightspec[i];
                if(knightspec[i]<64) MovableControl(tileon, knightspec[i]);
            }
        }

        //Bishop Case
        if(abs(p) == 4){
            Uint8 t = tileon-9;
            while(t<64&&t%8!=7){ //Up and left
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t -= 9;
            }

            t = tileon-7;
            while(t<64&&t%8!=0){ //Up and right
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t -= 7;
            }

            t = tileon+7;
            while(t<64&&t%8!=7){ //Down and left
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t += 7;
            }

            t = tileon+9;
            while(t<64&&t%8!=0){ //Down and right
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t += 9;
            }
        }

        //Rook Case
        if(abs(p) == 5){
            Uint8 t = tileon-1;
            while(t%8!=7){ //left side
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t -= 1;
            }

            t = tileon+1;
            while(t%8!=0){ //right side
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t += 1;
            }

            t = tileon-8;
            while(t<64){ //up side
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t -= 8;
            }
            
            t = tileon+8;
            while(t<64){ //down side
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t += 8;
            }
        }

        //Queen Case
        if(abs(p) == 6){
            Uint8 t = tileon-9;
            while(t<64&&t%8!=7){ //Up and left
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t -= 9;
            }
            t = tileon-7;
            while(t<64&&t%8!=0){ //Up and right
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t -= 7;
            }
            t = tileon+7;
            while(t<64&&t%8!=7){ //Down and left
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t += 7;
            }
            t = tileon+9;
            while(t<64&&t%8!=0){ //Down and right
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t += 9;
            }
            t = tileon-1;
            while(t%8!=7){ //left side
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t -= 1;
            }
            t = tileon+1;
            while(t%8!=0){ //right side
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t += 1;
            }
            t = tileon-8;
            while(t<64){ //up side
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t -= 8;
            }
            t = tileon+8;
            while(t<64){ //down side
                MovableControl(tileon, t);
                if(tileArr[t].piece!=0) t=99;
                t += 8;
            }
        }
    }

    //Repeat for King movement
    for(Uint8 tileon=0; tileon<64; tileon++){
        int p = tileArr[tileon].piece;
        bool s = tileArr[tileon].specialFlag;
        
        //King case, Must take place after everything else
        if(abs(p) == 1){
            int start = 0, end = 8;

            Uint8 kingspec[8] = {tileon-9, tileon-1, tileon+7, tileon-8, tileon+8, tileon-7, tileon+1, tileon+9};
            if(tileon%8==0) start = 3;
            if(tileon%8==7) end = 5;
            
            for(int i=start;i<end;i++){
                Uint8 tileto = kingspec[i];
                if(tileto<64 && p*tileArr[tileto].piece<=0 && tileArr[tileto].controlState[1+p]==0) tileArr[tileon].canMove[tileto] = 1;
            }
            
            //Castling
            if(s==1){
                if(tileon%8>2){
                    Uint8 tleft=tileon-tileon%8;
                    bool Lcastle=true;
                    if(abs(tileArr[tleft].piece)!=5 || tileArr[tleft].specialFlag!=1) Lcastle = false;
                    for(int i = tileon; i>tleft; i--) if(tileArr[i].controlState[1+p]!=0) Lcastle = false;
                    for(int i = tileon-1; i>tleft; i--) if(tileArr[i].piece!=0) Lcastle = false;
                    if(Lcastle) tileArr[tileon].canMove[tileon-2] = 1;
                }

                if(tileon%8<5){
                    Uint8 tright=tileon+7-tileon%8;
                    bool Rcastle=true;
                    if(abs(tileArr[tright].piece)!=5 || tileArr[tright].specialFlag!=1) Rcastle = false;
                    for(int i = tileon; i<tright; i++) if(tileArr[i].controlState[1+p]!=0) Rcastle = false;
                    for(int i = tileon+1; i<tright; i++) if(tileArr[i].piece!=0) Rcastle = false;
                    if(Rcastle) tileArr[tileon].canMove[tileon+2] = 1;
                }
            }
        }
    }
}

void MovableControl(int tileon, int tileto){
    int sign = -1;
    if(tileArr[tileon].piece>0) sign = 1;
    //Controls tileto
    tileArr[tileto].controlState[1-sign] = 1;
    //Can move to tileto
    if(sign*tileArr[tileto].piece<=0) tileArr[tileon].canMove[tileto] = 1;
}

int kingcheck() {
    int whitewin = 1;
    int blackwin = -1;
    for(int i=0; i<64; i++) if(tileArr[i].piece == -1) whitewin = 0;
    for(int i=0; i<64; i++) if(tileArr[i].piece == 1) blackwin = 0;
    return whitewin + blackwin;
}

Uint8 getTileOn(int x, int y){
    Uint8 res=0;
    if(16<x && x<784 && 16<y && y<784){
        res+=(x-16)/96;
        for(int i=0;i<8;i++) res+=(y-16)/96; //If I was a maniac, I'd make my tiles 64 units wide for easier bitshifting
    }
    else if(856<x && x<1154 && 176<y && y<624){
        res += 100;
        res+=(x-856)/150;
        for(int i=0;i<2;i++) res+=(y-176)/150;
    }
    else res = 99;
    //std::cout<<int(res)<<std::endl;
    return res;
}