#pragma once

#include "resource.h"

#include "iostream"
#include "cmath"
#include "string"
#include "windows.h"
#include "Tlhelp32.h"

#include "stack"

using namespace std;

bool LN(COLORREF clr, int ln)
{
	return ((clr>>8) & 0xff + (clr>>16) & 0xff)>unsigned(ln);
}

// my defines
#define TARGET_WINDOW_TITLE "É¨À×"
#define MAX_ROWNUM 24
#define MAX_COLNUM 30
#define DETECT_DEEPTH 30
#define MAX_SQUARE 100

#define XP_BASEX 15
#define XP_BASEY 100

#define EASY_ROW 9
#define EASY_COL 9
#define NORMAL_ROW 16
#define NORMAL_COL 16
#define HARD_ROW 16
#define HARD_COL 30

#define XPEASY_X 170
#define XPEASY_Y 255
#define XPNORMAL_X 282
#define XPNORMAL_Y 367
#define XPHARD_X 506
#define XPHARD_Y 368

#define XP_CHECK_POS_X 9
#define XP_CHECK_POS_Y 8
#define XP_CLR_0 0x00C0C0C0
#define XP_CLR_1 0x00FF0000
#define XP_CLR_2 0x00008000
#define XP_CLR_3 0x000000FF
#define XP_CLR_4 0x00800000
#define XP_CLR_5 0x00000080
#define XP_CLR_6 0x00808000
#define XP_CLR_7 0x00000000
#define XP_CLR_8 0x00808080
#define XP_CLR_OTHER 0x00FFFFFF

#define XP_CLICK_POS_X 8
#define XP_CLICK_POS_Y 8

#define XP_TITLE_X 3
#define XP_TITLE_Y 45

enum ProcessName { N_winmine, N_MineSweep, N_other};

#define MAX_ROW_NUM 24
#define MAX_COL_NUM 30

#define MIN_CLICK_DELAY 50

class MineSweeper
{
public:
	bool changed;
private:
	HWND outputWnd;
	HWND targetWnd;
	ProcessName proName;   // new
	RECT targetRect;
	RECT screenRect;

	HDC targetWndDC;

	bool running;
	int rowNum, colNum;
	int pieceLenX, pieceLenY;
	int baseX, baseY;
	int endX, endY;
	int screenBaseX, screenBaseY;

	// AI part
	bool fresh;
	bool done;
	int field[MAX_ROW_NUM][MAX_COL_NUM];
	bool open[MAX_ROW_NUM][MAX_COL_NUM];                    // tow unit wide edge
	bool isBomb[MAX_ROW_NUM][MAX_COL_NUM];

	// Advanced AI
	bool clear[MAX_ROW_NUM][MAX_COL_NUM];
	int bombNum[MAX_ROW_NUM][MAX_COL_NUM];
	int closeNum[MAX_ROW_NUM][MAX_COL_NUM];
	bool advClear[MAX_ROW_NUM][MAX_COL_NUM][8];     // left 1, right 1, up 1, down 1, ...2, ...2, ...2, ...2

	unsigned __int64 lastClickTime;
	stack <int> sta;

public:
	MineSweeper(HWND staticWnd)
	{	
		SetMapMode(GetWindowDC(0), MM_TEXT);

		changed = false;

		outputWnd = staticWnd;
		running = false;
		targetRect.bottom = targetRect.left = targetRect.right = targetRect.top = 0;

		srand(GetTickCount());
	}

	bool Running(void) { return running; }
	bool Begin(void);
	bool End(void);
	bool RealTime(void);
private:
	bool Lose(void);
	bool Win(void);
	ProcessName CheckProcess(void);
	bool InitTargetWindow(void);
	int CheckTargetWindow(void);
	HWND GetTargetWindow(void);
	bool MapMSField(void);
	bool OutputInfo(TCHAR *);
	int GetPieceStatu(int rowPos, int colPos);
	bool PieceClick(int rowPos, int colPos);

	// AI part
	bool AIProcess(void);
	bool InitField(void);
	bool FieldUpdate(int rowPos, int colPos, bool deal);
	bool FieldStatuDealer(int rowPos, int colPos, int statu);
};

bool MineSweeper::Begin(void)
{
	proName = CheckProcess();
	if (proName!=N_other && InitTargetWindow())
	{
		memset(open, 0, sizeof(open));
		memset(isBomb, 0, sizeof(isBomb));
		memset(clear, 0, sizeof(clear));
		memset(bombNum, 0, sizeof(bombNum));
		memset(advClear, 0, sizeof(advClear));

		int i,j;
		for (i=0; i<rowNum; i++)
			for (j=0; j<colNum; j++)
			{
				field[i][j] = -1;
				closeNum[i][j] = 8;
			}

		for (i=0; i<rowNum; i++)
			closeNum[i][0] = closeNum[i][colNum-1] = 5;
		for (j=0; j<colNum; j++)
			closeNum[0][j] = closeNum[rowNum-1][j] = 5;

		closeNum[0][0] = closeNum[0][colNum-1] = closeNum[rowNum-1][0] = closeNum[rowNum-1][colNum-1] = 3;

		for (i=0; i<rowNum; i++)
		{
			advClear[i][0][0]        = advClear[i][1][0]                                   = true;
			advClear[i][0][4]        = advClear[i][1][4]        = advClear[i][2][4]        = true;
			advClear[i][colNum-1][1] = advClear[i][colNum-2][1]                            = true;
			advClear[i][colNum-1][5] = advClear[i][colNum-2][5] = advClear[i][colNum-3][5] = true;
		}

		for (j=0; j<colNum; j++)
		{
			advClear[0][j][2]        = advClear[1][j][2]                                   = true;
			advClear[0][j][6]        = advClear[1][j][6]        = advClear[2][j][6]        = true;
			advClear[rowNum-1][j][3] = advClear[rowNum-2][j][3]                            = true;
			advClear[rowNum-1][j][7] = advClear[rowNum-2][j][7] = advClear[rowNum-3][j][7] = true;
		}

		if (!InitField())
			return false;

		changed = true;
		fresh = true;
		done = false;

		for (i=0; i<rowNum; i++)
		{
			for (j=0; j<colNum; j++)
				if (field[i][j]<9)
				{
					fresh = false;
					break;
				}
			if (fresh==false)
				break;
		}

		lastClickTime = GetTickCount64();

		while (!sta.empty())
			sta.pop();

		running = true;

		return true;
	}

	return false;
}

bool MineSweeper::End(void)
{
	if (running)
	{
		changed = true;
		running = false;
	}

	return true;
}

bool MineSweeper::RealTime(void)
{
	if (running)
		if (Lose() || Win() || !CheckTargetWindow() || !AIProcess())
			End();

	return true;
}

bool MineSweeper::Lose(void)
{
	if (proName==N_MineSweep)
		return FindWindow(NULL, _T("ÓÎÏ·Ê§°Ü"))!=NULL;
	else
		return false;
}

bool MineSweeper::Win(void)
{
	if (proName==N_winmine)
		return FindWindow(NULL, _T("ÓÎÏ·Ê¤Àû"))!=NULL;
	else
		return false;
}

bool MineSweeper::AIProcess(void)
{
	int i,j, iex,jex;
	int row, col;

	if (fresh)
	{
		fresh = false;

		row = rand()%rowNum;
		col = rand()%colNum;
		PieceClick(row, col);

		FieldUpdate(row, col, false);
	}

	if (done)
	{
		InitField();
	}
	else
		FieldUpdate(0, 0, true);

	done = true;

	for (row=0; row<rowNum; row++)
		for (col=0; col<colNum; col++)
		{
			if (!open[row][col] || clear[row][col])
				continue;

			if (closeNum[row][col]==field[row][col])
			{
				for (i=-1; i<=1; i++)
					for (j=-1; j<=1; j++)
					{
						if (!i && !j || row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum)
							continue;

						if (!open[row+i][col+j] && !isBomb[row+i][col+j])
						{
							isBomb[row+i][col+j] = true;

							for (iex=-1; iex<=1; iex++)
								for (jex=-1; jex<=1; jex++)
								{
									if (row+i+iex<0 || row+i+iex>=rowNum || col+j+jex<0 || col+j+jex>=colNum)
										continue;

									bombNum[row+i+iex][col+j+jex]++;
								}
						}
					}

				clear[row][col] = true;
			}
		}

	for (row=0; row<rowNum; row++)
		for (col=0; col<colNum; col++)
		{
			if (!open[row][col] || clear[row][col])
				continue;

			if (bombNum[row][col]==field[row][col])
			{
				for (i=-1; i<=1; i++)
					for (j=-1; j<=1; j++)
					{
						if (!i && !j || row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum)
							continue;

						if (open[row+i][col+j] || isBomb[row+i][col+j])
							continue;

						PieceClick(row+i, col+j);
						FieldUpdate(row+i, col+j, false);
						done = false;
					}

				clear[row][col] = true;
			}
		}

	int pbNum, pcNum, opcNum;
	for (row=0; row<rowNum; row++)
		for (col=0; col<colNum; col++)
		{
			if (!open[row][col] || clear[row][col])
				continue;

			if (!advClear[row][col][0] && open[row][col-1])             // left one
			{
				pbNum = pcNum = opcNum = 0;
				for (i=-1; i<=1; i++)
					for (j=1; j<=1; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row+i][col+j]) pbNum++;
						else if (!open[row+i][col+j])  pcNum++;

				pbNum = field[row][col-1] - field[row][col] + pbNum;

				for (i=-1; i<=1; i++)
					for (j=-1; j<=-1; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row+i][col-1+j])
							pbNum--;
						else if (!open[row+i][col-1+j])
							opcNum++;

				if (pbNum==-pcNum)
				{
					advClear[row][col][0] = true;
					for (i=-1; i<=1; i++)
						for (j=-1; j<=-1; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row+i][col-1+j] && !open[row+i][col-1+j])
							{
								PieceClick(row+i, col-1+j);
								FieldUpdate(row+i, col-1+j, false);
								done = false;
							}
				}
				else if (pbNum==opcNum)
				{
					advClear[row][col][0] = true;
					for (i=-1; i<=1; i++)
						for (j=-1; j<=-1; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row+i][col-1+j] && !open[row+i][col-1+j])
							{
								isBomb[row+i][col-1+j] = true;
								for (iex=-1; iex<=1; iex++)
									for (jex=-1; jex<=1; jex++)
									{
										if (row+i+iex<0 || row+i+iex>=rowNum || col-1+j+jex<0 || col-1+j+jex>=colNum)
											continue;
										bombNum[row+i+iex][col-1+j+jex]++;
									}
							}

				}
			}

			if (!advClear[row][col][1] && open[row][col+1])           // right one
			{
				pbNum = pcNum = opcNum = 0;
				for (i=-1; i<=1; i++)
					for (j=-1; j<=-1; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row+i][col+j]) pbNum++;
						else if (!open[row+i][col+j])  pcNum++;

				pbNum = field[row][col+1] - field[row][col] + pbNum;

				for (i=-1; i<=1; i++)
					for (j=1; j<=1; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row+i][col+1+j])
							pbNum--;
						else if (!open[row+i][col+1+j])
							opcNum++;

				if (pbNum==-pcNum)
				{
					advClear[row][col][1] = true;
					for (i=-1; i<=1; i++)
						for (j=1; j<=1; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row+i][col+1+j] && !open[row+i][col+1+j])
							{
								PieceClick(row+i, col+1+j);
								FieldUpdate(row+i, col+1+j, false);
								done = false;
							}
				}
				else if (pbNum==opcNum)
				{
					advClear[row][col][1] = true;
					for (i=-1; i<=1; i++)
						for (j=1; j<=1; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row+i][col+1+j] && !open[row+i][col+1+j])
							{
								isBomb[row+i][col+1+j] = true;
								for (iex=-1; iex<=1; iex++)
									for (jex=-1; jex<=1; jex++)
									{
										if (row+i+iex<0 || row+i+iex>=rowNum || col+1+j+jex<0 || col+1+j+jex>=colNum)
											continue;
										bombNum[row+i+iex][col+1+j+jex]++;
									}
							}
				}
			}

			if (!advClear[row][col][2] && open[row-1][col])            // up one
			{
				pbNum = pcNum = opcNum = 0;
				for (i=1; i<=1; i++)
					for (j=-1; j<=1; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row+i][col+j]) pbNum++;
						else if (!open[row+i][col+j])  pcNum++;

				pbNum = field[row-1][col] - field[row][col] + pbNum;

				for (i=-1; i<=-1; i++)
					for (j=-1; j<=1; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row-1+i][col+j])
							pbNum--;
						else if (!open[row-1+i][col+j])
							opcNum++;

				if (pbNum==-pcNum)
				{
					advClear[row][col][2] = true;
					for (i=-1; i<=-1; i++)
						for (j=-1; j<=1; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row-1+i][col+j] && !open[row-1+i][col+j])
							{
								PieceClick(row-1+i, col+j);
								FieldUpdate(row-1+i, col+j, false);
								done = false;
							}
				}
				else if (pbNum==opcNum)
				{
					advClear[row][col][2] = true;
					for (i=-1; i<=-1; i++)
						for (j=-1; j<=1; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row-1+i][col+j] && !open[row-1+i][col+j])
							{
								isBomb[row-1+i][col+j] = true;
								for (iex=-1; iex<=1; iex++)
									for (jex=-1; jex<=1; jex++)
									{
										if (row-1+i+iex<0 || row-1+i+iex>=rowNum || col+j+jex<0 || col+j+jex>=colNum)
											continue;
										bombNum[row-1+i+iex][col+j+jex]++;
									}
							}
				}
			}

			if (!advClear[row][col][3] && open[row+1][col])           // down one
			{
				pbNum = pcNum = opcNum = 0;
				for (i=-1; i<=-1; i++)
					for (j=-1; j<=1; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row+i][col+j]) pbNum++;
						else if (!open[row+i][col+j])  pcNum++;

				pbNum = field[row+1][col] - field[row][col] + pbNum;
				
				for (i=1; i<=1; i++)
					for (j=-1; j<=1; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row+1+i][col+j])
							pbNum--;
						else if (!open[row+1+i][col+j])
							opcNum++;

				if (pbNum==-pcNum)
				{
					advClear[row][col][3] = true;
					for (i=1; i<=1; i++)
						for (j=-1; j<=1; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row+1+i][col+j] && !open[row+1+i][col+j])
							{
								PieceClick(row+1+i, col+j);
								FieldUpdate(row+1+i, col+j, false);
								done = false;
							}
				}
				else if (pbNum==opcNum)
				{
					advClear[row][col][3] = true;
					for (i=1; i<=1; i++)
						for (j=-1; j<=1; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row+1+i][col+j] && !open[row+1+i][col+j])
							{
								isBomb[row+1+i][col+j] = true;
								for (iex=-1; iex<=1; iex++)
									for (jex=-1; jex<=1; jex++)
									{
										if (row+1+i+iex<0 || row+1+i+iex>=rowNum || col+j+jex<0 || col+j+jex>=colNum)
											continue;
										bombNum[row+1+i+iex][col+j+jex]++;
									}
							}
				}
			}

			if (!advClear[row][col][4] && open[row][col-2])         // left two
			{
				pbNum = pcNum = opcNum = 0;
				for (i=-1; i<=1; i++)
					for (j=0; j<=1; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row+i][col+j]) pbNum++;
						else if (!open[row+i][col+j])  pcNum++;

				pbNum = field[row][col-2] - field[row][col] + pbNum;

				for (i=-1; i<=1; i++)
					for (j=-1; j<=0; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row+i][col-2+j])
							pbNum--;
						else if (!open[row+i][col-2+j])
							opcNum++;
						
				if (pbNum==-pcNum)
				{
					advClear[row][col][4] = true;
					for (i=-1; i<=1; i++)
						for (j=-1; j<=0; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row+i][col-2+j] && !open[row+i][col-2+j])
							{
								PieceClick(row+i, col-2+j);
								FieldUpdate(row+i, col-2+j, false);
								done = false;
							}
				}
				else if (pbNum==opcNum)
				{
					advClear[row][col][4] = true;
					for (i=-1; i<=1; i++)
						for (j=-1; j<=0; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row+i][col-2+j] && !open[row+i][col-2+j])
							{
								isBomb[row+i][col-2+j] = true;
								for (iex=-1; iex<=1; iex++)
									for (jex=-1; jex<=1; jex++)
									{
										if (row+i+iex<0 || row+i+iex>=rowNum || col-2+j+jex<0 || col-2+j+jex>=colNum)
											continue;
										bombNum[row+i+iex][col-2+j+jex]++;
									}
							}
				}
			}

			if (!advClear[row][col][5] && open[row][col+2])         // right two
			{
				pbNum = pcNum = opcNum = 0;
				for (i=-1; i<=1; i++)
					for (j=-1; j<=0; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row+i][col+j]) pbNum++;
						else if (!open[row+i][col+j])  pcNum++;

				pbNum = field[row][col+2] - field[row][col] + pbNum;

				for (i=-1; i<=1; i++)
					for (j=0; j<=1; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row+i][col+2+j])
							pbNum--;
						else if (!open[row+i][col+2+j])
							opcNum++;

				if (pbNum==-pcNum)
				{
					advClear[row][col][5] = true;
					for (i=-1; i<=1; i++)
						for (j=0; j<=1; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row+i][col+2+j] && !open[row+i][col+2+j])
							{
								PieceClick(row+i, col+2+j);
								FieldUpdate(row+i, col+2+j, false);
								done = false;
							}
				}
				else if (pbNum==opcNum)
				{
					advClear[row][col][5] = true;
					for (i=-1; i<=1; i++)
						for (j=0; j<=1; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row+i][col+2+j] && !open[row+i][col+2+j])
							{
								isBomb[row+i][col+2+j] = true;
								for (iex=-1; iex<=1; iex++)
									for (jex=-1; jex<=1; jex++)
									{
										if (row+i+iex<0 || row+i+iex>=rowNum || col+2+j+jex<0 || col+2+j+jex>=colNum)
											continue;
										bombNum[row+i+iex][col+2+j+jex]++;
									}
							}
				}
			}

			if (!advClear[row][col][6] && open[row-2][col])          // up two
			{
				pbNum = pcNum = opcNum = 0;
				for (i=0; i<=1; i++)
					for (j=-1; j<=1; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row+i][col+j]) pbNum++;
						else if (!open[row+i][col+j])  pcNum++;

				pbNum = field[row-2][col] - field[row][col] + pbNum;

				for (i=-1; i<=0; i++)
					for (j=-1; j<=1; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row-2+i][col+j])
							pbNum--;
						else if (!open[row-2+i][col+j])
							opcNum++;

				if (pbNum==-pcNum)
				{
					advClear[row][col][6] = true;
					for (i=-1; i<=0; i++)
						for (j=-1; j<=1; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row-2+i][col+j] && !open[row-2+i][col+j])
							{
								PieceClick(row-2+i, col+j);
								FieldUpdate(row-2+i, col+j, false);
								done = false;
							}
				}
				else if (pbNum==opcNum)
				{
					advClear[row][col][6] = true;
					for (i=-1; i<=0; i++)
						for (j=-1; j<=1; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row-2+i][col+j] && !open[row-2+i][col+j])
							{
								isBomb[row-2+i][col+j] = true;
								for (iex=-1; iex<=1; iex++)
									for (jex=-1; jex<=1; jex++)
									{
										if (row-2+i+iex<0 || row-2+i+iex>=rowNum || col+j+jex<0 || col+j+jex>=colNum)
											continue;
										bombNum[row-2+i+iex][col+j+jex]++;
									}
							}
				}
			}

			if (!advClear[row][col][7] && open[row+2][col])         // down two
			{
				pbNum = pcNum = opcNum = 0;
				for (i=-1; i<=0; i++)
					for (j=-1; j<=1; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row+i][col+j]) pbNum++;
						else if (!open[row+i][col+j])  pcNum++;

				pbNum = field[row+2][col] - field[row][col] + pbNum;

				for (i=0; i<=1; i++)
					for (j=-1; j<=1; j++)
						if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
						else if (isBomb[row+2+i][col+j])
							pbNum--;
						else if (!open[row+2+i][col+j])
							opcNum++;
				
				if (pbNum==-pcNum)
				{
					advClear[row][col][6] = true;
					for (i=0; i<=1; i++)
						for (j=-1; j<=1; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row+2+i][col+j] && !open[row+2+i][col+j])
							{
								PieceClick(row+2+i, col+j);
								FieldUpdate(row+2+i, col+j, false);
								done = false;
							}
				}
				else if (pbNum==opcNum)
				{
					advClear[row][col][6] = true;
					for (i=0; i<=1; i++)
						for (j=-1; j<=1; j++)
							if (row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum) continue;
							else if (!isBomb[row+2+i][col+j] && !open[row+2+i][col+j])
							{
								isBomb[row+2+i][col+j] = true;
								for (iex=-1; iex<=1; iex++)
									for (jex=-1; jex<=1; jex++)
									{
										if (row+2+i+iex<0 || row+2+i+iex>=rowNum || col+j+jex<0 || col+j+jex>=colNum)
											continue;
										bombNum[row+2+i+iex][col+j+jex]++;
									}
							}
				}
			}
		}


	return true;
}

ProcessName MineSweeper::CheckProcess(void)
{
	HANDLE hSnap;
	hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 proEntry;
	proEntry.dwSize = sizeof(PROCESSENTRY32);

	if (!Process32First(hSnap, &proEntry))
	{
		CloseHandle(hSnap);
		return N_other;
	}

	const char t_name1[50]="winmine.exe", t_name2[50]="MineSweeper.exe";

	do
	{
		int cp;
		for (cp=0; cp<12 && proEntry.szExeFile[cp]==t_name1[cp]; cp++)
			;
		if (cp >= 12)
		{
			CloseHandle(hSnap);
			return N_winmine;
		}

		for (cp=0; cp<16 && proEntry.szExeFile[cp]==t_name2[cp]; cp++)
			;
		if (cp>=14)
		{
			CloseHandle(hSnap);
			return N_MineSweep;
		}
	}
	while (Process32Next(hSnap, &proEntry));

	CloseHandle(hSnap);
	return N_other;
}

bool MineSweeper::InitTargetWindow(void)
{
	if (!(targetWnd = GetTargetWindow()))
		return false;

	GetWindowRect(targetWnd, &targetRect);
	GetWindowRect(GetDesktopWindow(), &screenRect);

	if (targetRect.bottom>screenRect.bottom || targetRect.left<screenRect.left || targetRect.right>screenRect.right || targetRect.top<screenRect.top)
		return false;

	WINDOWPLACEMENT targetWinPlacement;
	targetWinPlacement.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(targetWnd, &targetWinPlacement);

	// check targetwindow: minisized? covered?
	if (targetWinPlacement.showCmd==SW_SHOWMINIMIZED)
		return false;

	SetWindowPos(targetWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	targetWndDC = GetWindowDC(targetWnd);

	int xSize, ySize;
	xSize = targetRect.right  - targetRect.left;
	ySize = targetRect.bottom - targetRect.top;

	if (proName==N_winmine)
	{
		pieceLenX = pieceLenY = 16;
		baseX = XP_BASEX, baseY = XP_BASEY;
		screenBaseX = targetRect.left+XP_BASEX;
		screenBaseY = targetRect.top+XP_BASEY;

		if (xSize==XPEASY_X && ySize==XPEASY_Y)
			rowNum = EASY_ROW, colNum = EASY_COL;
		else if (xSize==XPNORMAL_X && ySize==XPNORMAL_Y)
			rowNum = NORMAL_ROW, colNum = NORMAL_COL;
		else if (xSize==XPHARD_X && ySize==XPHARD_Y)
			rowNum = HARD_ROW, colNum = HARD_COL;

		return true;
	}
	else
		return false;
}

int MineSweeper::CheckTargetWindow(void)
{
	if (!(targetWnd = GetTargetWindow()))
		return false;

	RECT nowRect;
	GetWindowRect(targetWnd, &nowRect);

	if (nowRect.bottom!=targetRect.bottom || nowRect.left!=targetRect.left || nowRect.right!=targetRect.right || nowRect.top!=targetRect.top)
		return false;

	if (!BringWindowToTop(targetWnd))
		return false;

	return true;
}

HWND MineSweeper::GetTargetWindow(void)
{
	return FindWindow(NULL, _T(TARGET_WINDOW_TITLE));
}

bool MineSweeper::MapMSField(void)
{
#define MAX_LN1 int((pow(double(cy), 0.1)*5))
#define MAX_LN2 int((pow(double(cx), 0.1)*5))
#define MAX_LN3 20
#define MAX_LN4 int((pow(double(cy), 0.1)*10))

	static int cx=0, cy=0;
	if (targetRect.right-targetRect.left==cx && targetRect.bottom-targetRect.top==cy)
		return true;

	cx = targetRect.right-targetRect.left, cy = targetRect.bottom-targetRect.top;

	HDC targetDC = GetWindowDC(targetWnd);

	int x, y;
	

	for (x=cx/2, y=50; LN(GetPixel(targetDC, x, y), MAX_LN1) && y<cy; y++)
		;
	if (y>=cy)
		return false;

	baseY = y;

	for (x=cx/2, y=cy-20; LN(GetPixel(targetDC, x, y), MAX_LN1) && y>0; y--)
		;
	if (y<=0)
		return false;

	endY = y;

	x = 20, y = cy/2;
	while (GetPixel(targetDC, x, y)==RGB(0,0,0))
		x++;

	for (;LN(GetPixel(targetDC, x, y), MAX_LN2) && x<cx; x++)
		;
	if (x>=cx)
		return false;

	baseX = x;

	x = cx-20, y = cy/2;
	while (GetPixel(targetDC, x, y)==RGB(0,0,0))
		x--;

	for (; LN(GetPixel(targetDC, x, y), MAX_LN2) && x>0; x--)
		;
	if (x<=0)
		return false;

	endX = x;

	SetCursorPos(targetRect.left+endX, targetRect.top+endY);
	return true;

	COLORREF clr = GetPixel(targetDC, int(baseX+cy*0.05), int(baseY+cy*0.05));
	bool open = ((clr&0xff) + ((clr>>8) & 0xff) + ((clr>>16) & 0xff))>700;
	int len;

	if (open)
	{
		for (len = 12; LN(GetPixel(targetDC, baseX+len, baseY+len), MAX_LN3) && len<MAX_SQUARE; len++)
			;
		if (len>=MAX_SQUARE)
			return false;
	}
	else
	{
		POINT preCursorPos;
		GetCursorPos(&preCursorPos);

		int x = baseX-10, y = baseY+5;
		COLORREF arr[MAX_SQUARE];
		for (int i=0; i<MAX_SQUARE; i++)
			arr[i] = GetPixel(targetDC, x+i, y);

		SetCursorPos(targetRect.right-10, targetRect.bottom-10);
		PostMessage(targetWnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELONG(targetRect.right-10, targetRect.bottom-10));
		PostMessage(targetWnd, WM_LBUTTONUP, MK_LBUTTON, MAKELONG(targetRect.right-10, targetRect.bottom-10));
		SetCursorPos(targetRect.left+baseX+8, targetRect.top+baseY+8);

		int leftBound;
		int rightBound;
		for (leftBound=0; leftBound<MAX_SQUARE; leftBound++)
		{
			if (GetPixel(targetDC, x+leftBound, y)!=arr[leftBound])
				break;
		}
		if (leftBound>=MAX_SQUARE)
			return false;

		for (rightBound=leftBound+1; rightBound<MAX_SQUARE; rightBound++)
		{
			if (GetPixel(targetDC, x+rightBound, y)==arr[rightBound])
				break;
		}
		if (1 || rightBound>=MAX_SQUARE)
		{
			COLORREF endClr = GetPixel(targetDC, int(endX-cy*0.05), int(endY-cy*0.05));
			bool endOpen = ((endClr&0xff) + ((endClr>>8) & 0xff) + ((endClr>>16) & 0xff))>600;

			if (endOpen)
			{
				for (len = 10; LN(GetPixel(targetDC, endX-len, endY-len), MAX_LN3) && len<MAX_SQUARE; len++)
					;
				if (len>=MAX_SQUARE)
					return false;
			}
			else
			{
				for (len = 10; LN(GetPixel(targetDC, endX-len, endY-len), MAX_LN4) && len<MAX_SQUARE; len++)
					;
				if (len>=MAX_SQUARE)
					return false;
			}
		}
		else
			len = rightBound-leftBound;

		SetCursorPos(preCursorPos.x, preCursorPos.y);
	}

	rowNum = (endY-baseY) / len;
	colNum = (endX-baseX) / len;

	pieceLenY = int(double(endY-baseY) / double(rowNum));
	pieceLenX = int(double(endX-baseX) / double(colNum));

	SetCursorPos(int(targetRect.left+baseX+pieceLenX), int(targetRect.top+baseY+pieceLenY));

	ReleaseDC(targetWnd, targetDC);

	return true;
}

bool MineSweeper::OutputInfo(TCHAR *info)
{
	SetWindowText(outputWnd, info);
	return true;
}

int MineSweeper::GetPieceStatu(int rowPos, int colPos)
{
	if (rowPos<0 || colPos<0 || rowPos>=rowNum || colPos>=colNum)
		return -1;

	if (proName==N_winmine)
	{
		COLORREF clr = GetPixel(targetWndDC, baseX + colPos*pieceLenX + XP_CHECK_POS_X, baseY + rowPos*pieceLenY + XP_CHECK_POS_Y);
		COLORREF clr2;
		switch (clr)
		{
		case XP_CLR_0:
			clr2 = GetPixel(targetWndDC, baseX + colPos*pieceLenX + 1, baseY + rowPos*pieceLenY + 1);
			switch (clr2)
			{
			case XP_CLR_8:
				return 0;
			case XP_CLR_OTHER:
				return 9;
			default:
				return -1;
			}
		case XP_CLR_1:
			return 1;
		case XP_CLR_2:
			return 2;
		case XP_CLR_3:
			return 3;
		case XP_CLR_4:
			return 4;
		case XP_CLR_5:
			return 5;
		case XP_CLR_6:
			return 6;
		case XP_CLR_7:
			return 7;
		case XP_CLR_8:
			return 8;
		default:
			return 9;
		}
	}
	else
		return -1;
}

bool MineSweeper::PieceClick(int rowPos, int colPos)
{
	if (rowPos<0 || rowPos>=rowNum || colPos<0 || colPos>=colNum || open[rowPos][colPos])
		return true;

	POINT clickPoint;
	clickPoint.x = baseX + colPos*pieceLenX + XP_CLICK_POS_X - XP_TITLE_X;
	clickPoint.y = baseY + rowPos*pieceLenY + XP_CLICK_POS_Y - XP_TITLE_Y;

	// SetCursorPos(clickPoint.x + targetRect.left, clickPoint.y + targetRect.top);
	// PostMessage(targetWnd, WM_MOUSEMOVE,     0,          MAKELONG(clickPoint.x, clickPoint.y));
	PostMessage(targetWnd, WM_LBUTTONDOWN,   MK_LBUTTON, MAKELONG(clickPoint.x, clickPoint.y));
	PostMessage(targetWnd, WM_LBUTTONUP,     MK_LBUTTON, MAKELONG(clickPoint.x, clickPoint.y));

	return true;
}

bool MineSweeper::InitField(void)
{
	MSG targetWndMsg;
	while (PeekMessage(&targetWndMsg, targetWnd, WM_MOUSEFIRST, 0, PM_NOREMOVE))// && targetWndMsg.message==WM_LBUTTONUP)
		;
	
	int delay = GetTickCount();
	while (GetTickCount()-delay<2)
		;

	int row, col;
	int t;
	for (row = 0; row<rowNum; row++)
		for (col=0; col<colNum; col++)
		{
			if (open[row][col] || isBomb[row][col])
				continue;

			t = GetPieceStatu(row, col);
			if (t<0)
				return false;
			else if (field[row][col]!=t)
			{
				done = false;
				FieldStatuDealer(row, col, t);
			}
		}

	return true;
}

bool MineSweeper::FieldUpdate(int rowPos, int colPos, bool deal)
{
	int row, col;
	int statu;

	if (!deal)
	{
		sta.push(rowPos);
		sta.push(colPos);
	}
	else
	{
		MSG targetWndMsg;
		while (PeekMessage(&targetWndMsg, targetWnd, WM_MOUSEFIRST, 0, PM_NOREMOVE))// && targetWndMsg.message==WM_LBUTTONUP)
			;

		int delay = GetTickCount();
		while (GetTickCount()-delay<2)
			;

		while (!sta.empty())
		{
			col = sta.top();
			sta.pop();
			row = sta.top();
			sta.pop();

			statu = GetPieceStatu(row, col);
			if (statu!=field[row][col])
			{
				FieldStatuDealer(row, col, statu);

				if (row)          { sta.push(row-1); sta.push(col); }
				if (row<rowNum-1) { sta.push(row+1); sta.push(col); }
				if (col)          { sta.push(row);   sta.push(col-1); }
				if (col<colNum-1) { sta.push(row);   sta.push(col+1); }
			}
		}
	}

	return true;
}

bool MineSweeper::FieldStatuDealer(int row, int col, int statu)
{
	if (statu<0)
		return false;

	if (field[row][col]==statu)
		return true;

	int i,j;

	field[row][col] = statu;
	if (statu<9 && !open[row][col])
	{
		open[row][col] = true;
		for (i=-1; i<=1; i++)
			for (j=-1; j<=1; j++)
			{
				if (!i && !j || row+i<0 || row+i>=rowNum || col+j<0 || col+j>=colNum)
					continue;

				closeNum[row+i][col+j]--;
			}
	}

	return true;
}