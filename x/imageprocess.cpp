
//���漸��ͷ�ļ�ʹ�ø��ļ���imageprocess�����Կ�Խ��Ƭ���͵��ԣ�����ճ���󼴿������ڵ�Ƭ����
//���ڵ�Ƭ���͵����ϱ�д��ͬ��"environment.h"��"extVarContainer.h"
#include <stdint.h>		
#include "environment.h"		//ָʾ�˵�ǰ�Ļ���
#include "extVarContainer.h"	//����˵�Ƭ����һЩ����ת�Ƶ��ⲿ����������extern float speed;��������ĳ���

#ifdef RUN_ON_VS
#define _target_width  CAMERA_COLS 
#define _target_hight  CAMERA_ROWS 
extern void img_result_light(int row, int col, int r = 80, int g = 125, int b = 40);
extern void img_result_fill(uint8_t(*image)[_target_width]);
#endif // RUN_ON_VS

uint8_t(*img)[CAMERA_COLS];

void imageProcessOnChipAndOnVS(uint8_t (*image)[CAMERA_COLS])
{
	img = image;

#ifdef RUN_ON_VS
	img_result_fill(img);
#endif

	clean_shadow();//������Ӱ
	Edge_Detection();//Ѱ����
	wave_filter();//���صĴ�����
	current_status = Normal;//�������·��״�� Ϊ����
	/*��ͷ�ϰ�*************************************************/
	if (bt_barier == 1 && RunTime <= 5 && RunTime > 0)//bt_barierֻ��һ����ʼ��Ϊ0��RunTimeҲֻ��һ����ʼ��Ϊ0
	{
		if (Stepping_over_zebra())//�жϰ�����
			Stepping_over_zebra_flag = 1;
		else if (Stepping_over_zebra_flag == 1 && head_clear_flag == 1)//
		{
			Stepping_over_zebra_flag = 2;
			barrier_status_transform();//�ϰ�����Ϣ����
			zebra_find_distance = Distance;//�����ߵľ���
		}
	}
	/*����****************************************************/
	if (!Ring_FIND)//��־Ϊ0
	{
		Ring_Detection();//Բ�����
		if (Ring_FIND)
		{
			current_status = Ring_step_zero;
			current_ring_going_out_timing = 0;
			current_ring_timing = 0;
			addtional_ring_status = 0;
			A_special_restriction_for_ring_out = 0;
			memset(Road_Error_Recording, 0, sizeof(Road_Error_Recording));
		}
	}
	if (Ring_FIND)
	{
		Ring_Process();
	}
	if (current_status == Ring_just_out)
		goto path_end; //�����Ѿ������ʹ�����һ�Σ��������δ���
	/*����****************************************************/

	/*ʮ��****************************************************/
	if (!Ring_FIND)
	{
		if (Cross_Detection())
		{
			current_status = Cross;
			cross_process();
			check_cross_result();
		}
	}
	/*ʮ��****************************************************/

	/*ͨ�����ߴ���****************************************************/

	if (!Ring_FIND && current_status != Cross)
	{
		middle_processing();
	}
	/*ͨ�����ߴ���****************************************************/
	if (Terminal_zebra_flag == 1)
		goto path_end; //׼��ͣ��������ʶ������
	/*���ֱ��****************************************************/
	if (!Ring_FIND && current_status == Normal)
	{
		if (StraightRoad_Detection())
		{
			current_status = Straight_Perfect;
			if (check_hill_character())
			{
				current_status = Straight_Defect;
			}
		}
	}
	/*���ֱ��****************************************************/
	/*����µ�****************************************************/
	if (Distance > 1 && Hill_FIND == 0 && (current_status == Straight_Defect || current_status == Straight_Perfect))
	{
		if (Acc_Y < -25000)
		{
			Acc_Y_show = Acc_Y;
			current_status = Hill;
			Hill_FIND = 1;
			Hill_FIND_Distance = Distance;
			hill_to_be_confirm = 3;

			//��ȷ�ϵ��µ�������������ͼ��ȷ�ϵĻ��ᣬ
			//��ȫ������ͨ������ȡ���µ�״̬
		}
	}
	if (Hill_FIND == 1 && hill_to_be_confirm > 0) // hill_to_be_confirm > 0��δ���������µ�
	{
		hill_to_be_confirm--;
		if (current_status == Straight_Defect)
		{
			hill_to_be_confirm = -1; //���ȷ�ϣ��¸�ѭ������ִ���ϼ�if
			hill_override_barrier = 1;//ȷ���µ����򿪱�־��ֹ����ϰ�

		}
		else if (hill_to_be_confirm == 0)
		{
			Hill_FIND = 0; //hill_to_be_confirm��1����0���޷�ȷ��Ϊ�µ����˳�״̬
			Hill_FIND_Distance = 0;
			hill_to_be_confirm = 0;
		}
	}
	if (hill_override_barrier == 1 && Hill_FIND == 0)//���µ��Ѿ���ȥ���ոչ�ȥ����ʼ����Ƿ�����¿�ʼ����ϰ��������־
	{
		if (Distance > Hill_FIND_distance_memory + Hill_Distance[Hill_Cnt] + 0.5)
			hill_override_barrier = 0;
	}
	if (Hill_FIND == 1 && Distance > Hill_FIND_Distance + Hill_Distance[Hill_Cnt])
	{
		Hill_FIND_distance_memory = Hill_FIND_Distance;
		Hill_FIND = 0;
		Hill_Cnt++;
		Hill_FIND_Distance = 0;
		hill_to_be_confirm = 0;

	}
	/*����µ�****************************************************/
	/*����ϰ�****************************************************/
	if (!Barrier_existance_and_polarity && !Ring_FIND  && hill_override_barrier != 1 && current_status != Cross && current_status != Hill && Distance > 3 && (hill_to_be_confirm != -1))
	{
		//("\n����������ʼ�ж��ϰ�");
		int temp_result = decide_barrier_polarity();
		//("\ntemp_result%d", temp_result);
		if (temp_result == right_barrier)
			if (Is_left_line_straight_evo() && barrier_detaction_right())
			{
				Barrier_existance_and_polarity = 1;
				barrier_status = ba_phase_one;
				ObstacleRIGHT_middleProcess();
				BELL_ON;
				current_status = Straight_Perfect;
				barrier_start_distance = Distance;
				barrier_action_delayer = 0;
				gpio_set(LED_RED, 1);
			}
		if (temp_result == left_barrier)
			if (Is_right_line_straight_evo() && barrier_detaction_left())
			{
				Barrier_existance_and_polarity = 2;
				barrier_status = ba_phase_one;
				ObstacleLEFT_middleProcess();
				BELL_ON;
				current_status = Straight_Perfect;
				barrier_start_distance = Distance;
				barrier_action_delayer = 0;
				gpio_set(LED_RED, 1);
			}
	}
	if (Barrier_existance_and_polarity)
	{
		if (current_status == Cross)
		{
			Barrier_existance_and_polarity = 0;
			barrier_start_distance = 0;
			BELL_OFF;
		}

		if (barrier_action_delayer > barrier_exit)
		{
			gpio_set(LED_GREEN, 1);
		}

		if (Barrier_existance_and_polarity == 1 && barrier_action_delayer < barrier_exit)
		{ //���ϰ�
			gpio_set(LED_BLUE, 1);
			ObstacleRIGHT_middleProcess();
		}
		if (Barrier_existance_and_polarity == 2 && barrier_action_delayer < barrier_exit)
		{ //���ϰ�
			gpio_set(LED_BLUE, 1);
			ObstacleLEFT_middleProcess();
		}
		if (fabs(Distance - barrier_start_distance) > 4.0)
		{
			Barrier_existance_and_polarity = 0;
			barrier_start_distance = 0;
			BELL_OFF;
			gpio_set(LED_RED, 0);
			gpio_set(LED_GREEN, 0);
			gpio_set(LED_BLUE, 0);
		}

		int temp_result2 = check_barrier_status_evo(); //���״̬
		if (temp_result2 == 1)
		{
			Barrier_existance_and_polarity = 0;
			barrier_start_distance = 0;
			BELL_OFF;
			gpio_set(LED_RED, 0);
			gpio_set(LED_GREEN, 0);
			gpio_set(LED_BLUE, 0);
		}
	}
	/*����ϰ�****************************************************/
	/*��ⳤֱ��****************************************************/

	if (check_long_straight_line())
	{
		if (current_status == Straight_Perfect)
			current_status = Straight_Line_Very_Long;
		else
			current_status = Straight_Line_Very_Long_But_Not_Now;
	}
	/*��ⳤֱ��****************************************************/
	/*״̬������**************************************************************/
path_end: //�˴���ǩ������ת
	for (int p = record_num - 1; p > 0; p--)
	{
		history_status[p] = history_status[p - 1];
	}
	history_status[0] = current_status;
	smooth_middle();
	/*״̬������**************************************************************/
	/*��ʾ����**************************************************************/
#ifdef RUN_ON_VS
	for (int i = 0; i < CAMERA_H; i++) 
	{
		img_result_light(i, left[i],0,255,0);/*blue green red*/
		img_result_light(i, right[i],0,0,255);
		img_result_light(i, middle[i], 255, 0, 0);
	}
#endif // RUN_ON_VS


	/*��ʾ����**************************************************************/
}

void Edge_Detection(void)
{
	short line, column;									  //�洢����,�����ı���
	char left_check_finish_flag, right_check_finish_flag; //���������ɱ�־λ
	short center;

	for (line = RowMax; line >= RowMax - 2; line--)
	{
		left_check_finish_flag = 0;
		right_check_finish_flag = 0;
		center = 39;
		/*****************************************************************************************************/
		if (img[line][center] == THRESHOLD)
		{
			for (column = center; column >= 0; column--) //����м��Ϊ��ɫ���ֱ�������Ѱ�����ұ��ص�
				if (img[line][column] == 0)
				{
					left[line] = column + 1;//��Ϊ���е�����column�Ѿ��Լ�������+1
					left_check_finish_flag = 1;
					break;
				}
			for (column = center; column <= 79; column++)
				if (img[line][column] == 0)
				{
					right[line] = column - 1;//��Ϊ���е�����column�Ѿ��Լ�������-1
					right_check_finish_flag = 1;
					break;
				}
			if (left_check_finish_flag == 0) //���û���ҵ����ұ��أ���0��79�������ұ���
				left[line] = 0;
			if (right_check_finish_flag == 0)
				right[line] = 79;
		}
		/*****************************************************************************************************/
		else //����м�Ϊ��ɫ���������ѭ��
		{
			int center_temp = 0; //��ȡ����
			int HighPointCount = 0;
			int HighPoint = 0;
			for (column = 1; column <= 78; column++)
			{
				if (img[line][column] == THRESHOLD)
				{
					HighPointCount += 1;//ͳ��ͼ����һ���ڶ��ٸ��׵㣬��������ĵ�
					HighPoint = HighPoint + column;//�߶��ۼӣ������
				}
			}
			if (HighPointCount != 0)
			{
				center_temp = HighPoint / HighPointCount;//������Ե׵�ƽ���߶ȣ�������
			}
			else
			{
				center_temp = 0;
			}
			for (column = center_temp; column >= 0; column--) //����м��Ϊ��ɫ���ֱ�������Ѱ�����ұ��ص�
				if (img[line][column] == 0)
				{
					left[line] = column + 1;
					left_check_finish_flag = 1;
					break;
				}
			for (column = center_temp; column <= 79; column++)
				if (img[line][column] == 0)
				{
					right[line] = column - 1;
					right_check_finish_flag = 1;
					break;
				}
			if (left_check_finish_flag == 0) //���û���ҵ����ұ��أ���0��79�������ұ���
				left[line] = 0;
			if (right_check_finish_flag == 0)
				right[line] = 79;
		}
		////////////////
		////////////////
		////////////////ǰ���б������߸��ŵ����
		if (right[line] - left[line] < 10 && right[line] - left[line] > -10)
		{
			left_check_finish_flag = 0;
			right_check_finish_flag = 0;
			for (column = 0; column <= 78; column++) //����м��Ϊ��ɫ���ֱ�������Ѱ�����ұ��ص�//�ֱ��������м�Ѱ����
				if (img[line][column] == 0 && img[line][column + 1] == THRESHOLD)
				{
					left[line] = column + 1;
					left_check_finish_flag = 1;
					break;
				}
			for (column = 79; column >= 1; column--)
				if (img[line][column] == 0 && img[line][column - 1] == 0)
				{
					right[line] = column - 1;
					right_check_finish_flag = 1;
					break;
				}
			if (left_check_finish_flag == 0) //���û���ҵ����ұ��أ���0��79�������ұ���
				left[line] = 0;
			if (right_check_finish_flag == 0)
				right[line] = 79;
		}
	}

	/**************************************************************************************************************************/

	for (; line >= RowMin; line--)
	{
		left_check_finish_flag = 0;
		right_check_finish_flag = 0;
		if (left[line + 1] != -1) //��һ�������������,����߲���ȫ�ڵ�
		{
			column = left[line + 1];	//����һ�б���ֵ������һ��
			if (img[line][column] == 0) //����˴�ֵΪ�ڣ������������ұߣ���������Ѱ��
			{
				for (; column <= 79; column++)
					if (img[line][column] == THRESHOLD)
					{
						left[line] = column;
						left_check_finish_flag = 1;
						break;
					}
				//����ѭ������

				if (left_check_finish_flag != 1)
				{
					left[line] = -1;
					left_check_finish_flag = 0;
				}
				//������ұ�־λΪ0��˵������ȫ��
			}
			else //����˴�ֵΪ��,����Ѱ��
			{
				for (; column >= 0; column--)
					if (img[line][column] == 0)
					{
						left[line] = column + 1;
						left_check_finish_flag = 1;
						break;
					}
				if (left_check_finish_flag != 1)
				{
					left[line] = 0;
					left_check_finish_flag = 0;
				} //������ұ�־λΪ0��˵������ȫ��
			}
		}
		else
			left[line] = -1; //-1��ʾȫ�Ǻڵģ��������Ӧ�ó����ں�����
		//�˴�Ӧ����bug�����һ��ȫ�ڣ���ô�������ϵ�ͼ��ȫ��ʧЧ���������ʺ�С�ɺ���
							 /*******************************************************************************************************************/
		if (right[line + 1] != -1)
		{
			column = right[line + 1];   //����һ�б���ֵ������һ��
			if (img[line][column] == 0) //����˴�ֵΪ��,������
			{
				for (; column >= 0; column--)
					if (img[line][column] == THRESHOLD)
					{
						right[line] = column;
						right_check_finish_flag = 1;
						break;
					}
				//����ѭ������
				if (right_check_finish_flag != 1)
				{

					right[line] = -1;
					right_check_finish_flag = 0;
				}
				//������ұ�־λΪ0��˵������ȫ��
			}
			else //����˴�ֵΪ��, ������
			{
				for (; column <= 79; column++)
					if (img[line][column] == 0)
					{
						right[line] = column - 1;
						right_check_finish_flag = 1;
						break;
					}
				if (right_check_finish_flag != 1)
				{
					right[line] = 79;
					right_check_finish_flag = 0;
				} //������ұ�־λΪ0��˵������ȫ��
			}
		}
		else
			right[line] = -1;
	}
}
//�򵥵��˲�
void wave_filter(void)
{
	short line;

	for (line = RowMax; line >= RowMin; line--)
	{
		if (left[line] == -1 || right[line] == -1)
		{
			for (; line >= RowMin; line--)
			{
				right[line] = -1;
				left[line] = -1;
			}
			break;
		}
	}//���ĳ��ȫ�ڣ������������ϵ�ͼ��Ϊȫ�ڲ������ж�
	for (line = RowMax; line >= RowMin; line--)
	{
		if ((right[line] <= left[line]) && (left[line] != -1))
		{
			for (; line >= RowMin; line--)
			{
				right[line] = -1;
				left[line] = -1;
			}
			break;
		}
	}//���ĳ��Ѱ����������ұ��ص��ұߣ�����ͼ��ȫ�ڲ������ж�
}
//�򵥵�������
void middle_processing(void)
{
	short line;
	for (line = RowMax; line >= RowMin; line--)
	{
		if (left[line] != -1 && left[line] != 0 && right[line] != 79 && line < RowMax) //����ȫ��������û�ж���
			middle[line] = (right[line] + left[line]) / 2;
		else if (left[line] != -1 && line == RowMax) //����ȫ�� �� �����
			middle[line] = (right[line] + left[line]) / 2;
		else if (left[line] != -1 && left[line] == 0 && right[line] != 79 && line < RowMax)
		{
			middle[line] = right[line] - (right[line + 1] - middle[line + 1]) * BendFixRatio / 1000.0; //break;
		}
		else if (left[line] != -1 && left[line] != 0 && right[line] == 79 && line < RowMax)
		{
			middle[line] = left[line] + (middle[line + 1] - left[line + 1]) * BendFixRatio / 1000.0; //break;
		}
		else if (left[line] != -1 && left[line] == 0 && right[line] == 79 && line < RowMax)
		{
			middle[line] = middle[line + 1];
		}
		else if (left[line] == -1)
		{
			middle[line] = middle[line + 1];
			if (line < Control_Line[0])
				break;
		}

		if (left[line] != -1 && left[line] != 0 && right[line] != 79 && line < RowMax - 5 && right[line + 1] == 79 && right[line + 2] == 79 && right[line + 3] == 79) //���߶���
		{
			middle[line] = left[line] + (middle[line + 1] - left[line + 1]) * BendFixRatio / 1000.0; //break;
		}
		if (left[line] != -1 && left[line] != 0 && right[line] != 79 && line < RowMax - 5 && right[line + 2] == 79 && right[line + 3] == 79 && right[line + 4] == 79) //���߶���
		{
			middle[line] = left[line] + (middle[line + 1] - left[line + 1]) * BendFixRatio / 1000.0; //break;
		}

		if (left[line] != -1 && left[line] != 0 && right[line] != 79 && line < RowMax - 5 && left[line + 1] == 0 && left[line + 2] == 0 && left[line + 3] == 0) //�󵥱߶���
		{
			middle[line] = right[line] - (right[line + 1] - middle[line + 1]) * BendFixRatio / 1000.0; //break;
		}
		if (left[line] != -1 && left[line] != 0 && right[line] != 79 && line < RowMax - 5 && left[line + 2] == 0 && left[line + 3] == 0 && left[line + 4] == 0) //�󵥱߶���
		{
			middle[line] = right[line] - (right[line + 1] - middle[line + 1]) * BendFixRatio / 1000.0; //break;
		}
	}
}
void Ring_right_middle_processing(void)
{
	short line;
	int line_destiny;
	for (line = RowMax; line >= RowMin; line--)
	{
		if (left[line] != -1 && line == 59)
			middle[line] = (left[line] + right[line]) / 2;
		if (left[line] != -1 && line == 58)
			middle[line] = (left[line] + right[line]) / 2;
		if (left[line] != -1 && line == 57)
			middle[line] = (left[line] + right[line]) / 2;
		if (left[line] != -1 && line == 56)
			middle[line] = (left[line] + right[line]) / 2;
		if (left[line] != -1 && line == 55)
			middle[line] = (left[line] + right[line]) / 2;
		if (left[line] >= left[line + 1] && left[line - 1] <= left[line] && left[line - 2] < left[line])
		{
			line_destiny = line;
			break;
		}
	}

	for (line = 54; line > line_destiny; line--)
	{
		if (left[line] != -1)
			middle[line] = (left[line] + right[line]) / 2;
		else
		{
			middle[line] = middle[line + 1];
			break;
		}
	}
	for (line = line_destiny; line >= RowMin; line--)
	{
		if (left[line] != -1)
			middle[line] = right[line] - (right[line_destiny] - left[line_destiny]) / 2;
		else
		{
			middle[line] = middle[line + 1];
			break;
		}
	}
}

float Middle_Err_Filter(float middle_err) //����ƫ���˲�
{
	float Middle_Err_Fltered;
	static float Pre3_Error[4];
	Pre3_Error[3] = Pre3_Error[2];
	Pre3_Error[2] = Pre3_Error[1];
	Pre3_Error[1] = Pre3_Error[0];
	Pre3_Error[0] = middle_err;
	Middle_Err_Fltered = Pre3_Error[0] * 0.4 + Pre3_Error[1] * 0.3 + Pre3_Error[2] * 0.2 + Pre3_Error[3] * 0.1;
	return Middle_Err_Fltered;
}

uint8 catch_road_j()
{
	uint8 max_j = 0, max_cnt = 0, cnt;
	for (uint8 j = 20; j < 60; j++)
	{
		cnt = 0;
		for (uint8 i = 0; i < CAMERA_H; i++)
		{
			if (img[i][j] == THRESHOLD)
				cnt++;
		}

		if (cnt > max_cnt)
		{
			if (img[59][j] == THRESHOLD && img[58][j] == THRESHOLD && img[58][j - 1] == THRESHOLD && img[58][j + 1] == THRESHOLD)
			{
				max_j = j;
				max_cnt = cnt;
			}
		}
	}
	return max_j;
}

uint8 mark[CAMERA_H];
void cross_process()
{
	uint16 status[CAMERA_H][3];
	memset(status, 0, sizeof(status));
	memset(mark, 0, sizeof(mark));
	uint8 basic_j = catch_road_j();
	uint8 i, j;

	uint8 len = 0;
	uint8 len_max_guess = 0;
	for (i = CAMERA_H - 1; i > 4; i--)
	{

		len = 0;

		j = basic_j;

		while (img[i][j] == THRESHOLD && j > 0)
		{
			j--;
		}
		status[i][1] = j;

		j = basic_j;

		while (img[i][j] == THRESHOLD && j < CAMERA_W)
		{
			j++;
		}
		status[i][2] = j;

		len = status[i][2] - status[i][1];
		////("\nline%d: %d <=    %d   <= %d", i, road_len_min[i], len, road_len_max[i]);

		if (len >= road_len_min[i] && len <= road_len_max[i]&& (len_max_guess ==0||i>CAMERA_H-4||(len_max_guess>0&& len< len_max_guess)))
		{
			if (
				(
				(status[i + 2][1] - status[i][1]) > 3 
				|| (status[i + 3][1] - status[i][1]) > 7	
				|| (status[i][2] - status[i + 2][2]) > 3
				) && status[i + 2][0] != 0 && i < 56)
			{
				status[i][0] = 0;
			}
			else
			{
				status[i][0] = len;
				if (i == CAMERA_H - 1) len_max_guess = len;
		
				mark[i] = 1;
#ifdef RUN_ON_VS
				for (int tp = status[i][1]; tp <= status[i][2]; tp++) {
					img_result_light(i, tp, 60, 60, 60);
				}
#endif

				j = basic_j;
				while (img[i][j] == THRESHOLD && j > 0)
				{
					j--;
				}

				j = basic_j;
				while (img[i][j] == THRESHOLD && j < CAMERA_W - 1)
				{
					j++;
				}

				middle[i] = (status[i][1] + status[i][2]) / 2;
			}
		}
	}
	for (i = CAMERA_H - 1; i > 4; i--)
	{
		if (status[i][0] == 0)
		{
			uint8 it = i, iup = i, idown = 0;
			while (status[it][0] == 0 && abs(it - i) < 40 && it > 0)
			{
				it--;
			} //������
			if (status[it][0] != 0)
			{
				iup = it;
				it = i;
				while ((status[it][0] == 0 && it < CAMERA_H - 1 && abs(it - i) < 32) || mark[it] == 0 )
				{
					it++;
				}
				if (status[it][0] != 0)
				{
					idown = it;
				}
				else
				{
					idown = 59;
					status[59][1] = 15;
					status[59][2] = 65;
				}
				status[i][1] = status[iup][1] * (idown - i) + status[idown][1] * (i - iup);
				status[i][1] = division45((float)status[i][1] / (idown - iup));
				if (status[i][1] < 0)
					status[i][1] = 0;

				status[i][2] = status[iup][2] * (idown - i) + status[idown][2] * (i - iup);
				status[i][2] = division45((float)status[i][2] / (idown - iup));
				if (status[i][2] > 79)
					status[i][2] = 79;
				int q1, q2;
				q1 = status[i][1];
				q2 = status[i][2];
				status[i][0] = q2 - q1;
#ifdef RUN_ON_VS
				img_result_light(i, q1, 0, 125, 125);
				img_result_light(i, q2, 125, 0, 125);
#endif
				////////////////set center//////////////

				middle[i] = (q1 + q2) / 2;
			}
			else
			{ //�����Ҳ�����
			}
		}
	}
	int no_result_flag = 1;
	for (int p = 10; p < 40; p++)
	{
		if (status[p][0] != 0)//ȷ���Ƿ������в�λ����������
		{
			no_result_flag = 0;
			break;
		}
	}
	if (no_result_flag)
	{
		for (i = CAMERA_H - 1; i > 4; i--)
		{
			len = status[i][2] - status[i][1];
			if (len >= road_len_min[i] && len <= 40)
			{
				if (((status[i + 2][1] - status[i][1]) > 3 || (status[i][2] - status[i + 2][2]) > 3) && status[i + 2][0] != 0 && i < 56)
				{
					status[i][0] = 0;
				}
				else
				{
					status[i][0] = len;

					j = basic_j;
					while (img[i][j] == THRESHOLD && j > 0)
					{
						j--;
					}

					j = basic_j;
					while (img[i][j] == THRESHOLD && j < CAMERA_W - 1)
					{
						j++;
					}

					middle[i] = (status[i][1] + status[i][2]) / 2;
				}
			}
		}
		int row = 59;
		PosType row_middle[60];
		int cnt = 0;
		for (row = 59; row > 0; row--)
		{
			if (status[row][0] > 0)
			{

				row_middle[cnt].x = row;
				row_middle[cnt].y = middle[row];
				cnt++;
			}
		}
		int row_min_normal = row;
		if (cnt == 0)
			return;

		LeastSquareMethod(row_middle, cnt);

		for (row = 59; row > 0; row--)
		{
			middle[row] = k * row + d;
		}
	}
	else if (status[20][0] == 0)
	{

		int row = 59;
		PosType row_middle[60];
		int cnt;
		for (cnt = 0; status[row][0] > 0 && row > 0; row--, cnt++)
		{
			row_middle[cnt].x = row;
			row_middle[cnt].y = middle[row];
		}
		int row_min_normal = row;
		if (cnt == 0)
			return;

		LeastSquareMethod(row_middle, cnt);

		for (row = row_min_normal; row > 0; row--)
		{
			middle[row] = k * row + d;
		}
	}
}

uint8 Cross_Detection(void)
{
	short line;							   //������Ϊ�в��ҵı�����
	short Cross_Find_Line;				   //��¼�ҵ�ʮ�ֵı�־��
	short cross_leftdown, cross_rightdown; //��¼�����������۵�
	uint8 left_flag = 0, right_flag = 0;   //��Ϊ�Ƿ��ҵ��۵�ı�־
	uint8 cross_flag = 0;
	Cross_FIND = 0;
	for (line = RowMax - 5; line >= RowMin + 5; line--) //����ȫ���� ��Ϊ����ʮ�ֵ�һ�ַ���
	{
		if (left[line] <= 1 && right[line] >= 78)
		{
			cross_flag = 1;
			break;
		}
		else if (left[line] <= 7 && right[line] >= 72 && line <= 30)
		{
			cross_flag = 1;
			break;
		}
		else if (left[line] == -1)
			break;
	}
	//�ж�ʮ�ֵ�����
	Cross_Find_Line = line; //���
	return cross_flag;
}

/****************������ײ����е���Ϣ����Ҫ��ʱ������õ�*******************/
void Edge_Detection_Further(void)
{
	short line, column;									  //�洢����,�����ı���
	char left_check_finish_flag, right_check_finish_flag; //���������ɱ�־λ
	for (line = RowMax + 1; line <= 59; line++)
	{
		left_check_finish_flag = 0;
		right_check_finish_flag = 0;
		/*���������*/
		column = left[line - 1];
		if (img[line][column] == THRESHOLD)
		{
			for (; column >= 0; column--)
			{
				if (img[line][column] == 0)
				{
					left_check_finish_flag = 1;
					left[line] = column + 1;
					break;
				}
			}
			if (left_check_finish_flag == 0)
				left[line] = 0;
		}
		else
		{
			for (; column <= 79; column++)
			{
				if (img[line][column] == THRESHOLD)
				{
					left_check_finish_flag = 1;
					left[line] = column;
					break;
				}
			}
		} //�˴��������Ҳ��������
		  /*�����ұ���*/
		column = right[line - 1];
		if (img[line][column] == THRESHOLD)
		{
			for (; column <= 79; column++)
			{
				if (img[line][column] == 0)
				{
					right[line] = column - 1;
					right_check_finish_flag = 1;
					break;
				}
				if (right_check_finish_flag == 0)
					right[line] = 79;
			}
		}
		else
		{
			for (; column >= 0; column--)
			{
				if (img[line][column] == 255)
				{
					right[line] = column;
					right_check_finish_flag = 1;
					break;
				}
			}
		} //�˴��������Ҳ��������
	}
}
//�����߽�����ʾ
void LeastSquareMethod(PosType *data, uint8_t size)
{
	uint8 i;
	int xsum = 0;
	int x2sum = 0;
	int ysum = 0;
	int xysum = 0;

	for (i = 0; i < size; i++)
	{
		xsum += data[i].x;//x���
		x2sum += (data[i].x) * (data[i].x);//x��ƽ����
		ysum += data[i].y;//y���
		xysum += (data[i].x) * (data[i].y);//x*y���
	}

	if (size * x2sum - xsum * xsum == 0)//�ų��������
	{
		k = 0;
		d = 0;
	}
	else
	{ 	
		k = (float)(size * xysum - xsum * ysum) / (float)(size * x2sum - xsum * xsum);
		d = (float)(x2sum * ysum - xysum * xsum) / (float)(size * x2sum - xsum * xsum);
	}

}
void Stop_Detection(void) //ֹͣ�߼��
{
	short line, column;
	int stop_detection_count = 0;
	line = 50;
	for (column = left[line]; column <= right[line]; column++)
	{
		if (img[line][column] == THRESHOLD && img[line][column + 1] == 0)
			stop_detection_count++;
	}
	if (stop_detection_count >= 5 && Stop_cnt == 0)
	{
		Stop_cnt = 1;
	}
	if (stop_detection_count <= 3 && Stop_cnt == 1)
	{
		STOP = 1;
		Terminal_zebra_flag = 1;
	}
}

int Straight_Detection(void)
{
	//����ڻ���
	int BLACKRING = 0;
	int line, column;
	for (line = RowMax - 10; line >= RowMin + 10; line--)
	{
		int Ring_line_flag[2] = { 0 };
		for (column = left[line]; column <= right[line]; column++)
		{
			if (img[line][column] == THRESHOLD && img[line][column + 1] == 0)
			{
				Ring_line_flag[0] = column;
				break;
			}
		}
		for (; column <= right[line]; column++)
		{
			if (img[line][column] == 0 && img[line][column + 1] == THRESHOLD)
			{
				Ring_line_flag[1] = column;
				break;
			}
		}
		if (((Ring_line_flag[1] - Ring_line_flag[0]) > 30) && (Ring_line_flag[0] - left[line]) > 10 && (right[line] - Ring_line_flag[1]) > 10)
		{
			//   //("line%d left %d\t right %d\n",line,Ring_line_flag[0],Ring_line_flag[1]);
			BLACKRING = 1;
			break; //Բ���Ĵ��Բ
		}
	}

	//��ֱ�����
	float temp_k_more[2][3] = { 0 };
	float temp_k[2] = { 0 };
	float temp_err[2] = { 0 };
	temp_k[0] = (left[RowMax - 35] - left[RowMax]) / 35.0;
	temp_k_more[0][0] = (left[RowMax - 30] - left[RowMax - 20]) / 10.0;
	temp_k_more[0][1] = (left[RowMax - 20] - left[RowMax - 10]) / 10.0;
	temp_k_more[0][2] = (left[RowMax - 10] - left[RowMax]) / 10.0;

	temp_k[1] = (right[RowMax - 35] - right[RowMax]) / 35.0;
	temp_k_more[1][0] = (right[RowMax - 30] - right[RowMax - 20]) / 10.0;
	temp_k_more[1][1] = (right[RowMax - 20] - right[RowMax - 10]) / 10.0;
	temp_k_more[1][2] = (right[RowMax - 10] - right[RowMax]) / 10.0;

	int i = 0;
	float YUZHI = 1;
	for (i = 1; i < 35; i++)
	{
		float temp = 0;
		if ((left[RowMax] + temp_k[0] * i - left[RowMax - i]) < YUZHI && (left[RowMax] + temp_k[0] * i - left[RowMax - i]) > -YUZHI)
			temp = 0;
		else
			temp = (left[RowMax] + temp_k[0] * i - left[RowMax - i]);
		temp_err[0] = temp_err[0] + temp;
	}
	for (i = 1; i < 35; i++)
	{
		float temp = 0;
		if ((right[RowMax] + temp_k[1] * i - right[RowMax - i]) < YUZHI && (right[RowMax] + temp_k[1] * i - right[RowMax - i]) > -YUZHI)
			temp = 0;
		else
			temp = (right[RowMax] + temp_k[1] * i - right[RowMax - i]);
		temp_err[1] = temp_err[1] + temp;
	}

	if (temp_err[0] > -8 && temp_err[0] < 8 && temp_err[1] > -8 && temp_err[1] < 8 && right[RowMax] != 79 && right[RowMax - 1] != 79 && right[RowMax - 2] != 79 && left[RowMax] > 2 && left[RowMax - 1] > 2 && left[RowMax - 2] > 2 && BLACKRING == 0)
	{
		return 1;
	}
	else
		return 0;
}

int Ring_Turn_Detection(void)
{
	static int cnt = 0;

	if (Ring_Exit_Deriction[Ring_cnt] == 0)
	{
		if (Turn_Speed > 10 && cnt == 1)
		{
			cnt = 0;
			return 1;
		}

		else
		{
			cnt = 1;
			return 0;
		}
	}
	else
	{
		if (Turn_Speed < -10 && cnt == 1)
		{
			cnt = 0;
			return 1;
		}

		else
		{
			cnt = 1;
			return 0;
		}
	}
}

int BaToMid = 7;

void ObstacleRIGHT_middleProcess(void)
{
	int line;
	for (line = RowMax; line > RowMin; line--)
	{
		if (left[line] != -1)
		{

			if (Distance < 3)
			{
				if (line == Control_Line[0])
				{
					middle[line] = left[line] + (right[line] - left[line]) * bt_to_mid / 32.0;
				}
				else if (line == Control_Line[1])
				{
					middle[line] = left[line] + (right[line] - left[line]) * bt_to_mid / 32.0;
				}
				else if (line == Control_Line[2])
				{
					middle[line] = left[line] + (right[line] - left[line]) * bt_to_mid / 32.0;
				}
				else if (line == Control_Line[3])
				{
					middle[line] = left[line] + (right[line] - left[line]) * bt_to_mid / 32.0;
				}
				else
					middle[line] = left[line] + (right[line] - left[line]) * bt_to_mid / 32.0;



			}
			else
			{

				if (line == Control_Line[0])
				{
					middle[line] = left[line] + (right[line] - left[line]) * BaToMid / 32.0;
				}
				else if (line == Control_Line[1])
				{
					middle[line] = left[line] + (right[line] - left[line]) * BaToMid / 32.0;
				}
				else if (line == Control_Line[2])
				{
					middle[line] = left[line] + (right[line] - left[line]) * BaToMid / 32.0;
				}
				else if (line == Control_Line[3])
				{
					middle[line] = left[line] + (right[line] - left[line]) * BaToMid / 32.0;
				}
				else
					middle[line] = left[line] + (right[line] - left[line]) * BaToMid / 32.0;

			}


		}
		else
		{
			middle[line] = middle[line + 1];
		}
	}
}

void ObstacleLEFT_middleProcess(void)
{
	int line;
	for (line = RowMax; line > RowMin; line--)
	{
		if (left[line] != -1)
		{

			if (Distance < 3)
			{

				if (line == Control_Line[0])
				{
					middle[line] = right[line] - (right[line] - left[line]) * bt_to_mid / 32.0;
				}
				else if (line == Control_Line[1])
				{
					middle[line] = right[line] - (right[line] - left[line]) * bt_to_mid / 32.0;
				}
				else if (line == Control_Line[2])
				{
					middle[line] = right[line] - (right[line] - left[line]) * bt_to_mid / 32.0;
				}
				else if (line == Control_Line[3])
				{
					middle[line] = right[line] - (right[line] - left[line]) * bt_to_mid / 32.0;
				}
				else
					middle[line] = right[line] - (right[line] - left[line]) * bt_to_mid / 32.0;

			}
			else
			{


				if (line == Control_Line[0])
				{
					middle[line] = right[line] - (right[line] - left[line]) * BaToMid / 32.0;
				}
				else if (line == Control_Line[1])
				{
					middle[line] = right[line] - (right[line] - left[line]) * BaToMid / 32.0;
				}
				else if (line == Control_Line[2])
				{
					middle[line] = right[line] - (right[line] - left[line]) * BaToMid / 32.0;
				}
				else if (line == Control_Line[3])
				{
					middle[line] = right[line] - (right[line] - left[line]) * BaToMid / 32.0;
				}
				else
					middle[line] = right[line] - (right[line] - left[line]) * BaToMid / 32.0;


			}
		}
		else
		{
			middle[line] = middle[line + 1];
		}
	}
}

//���Ϊ2
int division45(float kfloat)
{ //��������
	return (int)(kfloat + 0.5);
}

void smooth_middle()
{
	for (uint8 cc = 0; cc < 2; ++cc)
	{
		if (Control_Line_WE[cc] != 0)
		{
			middle[Control_Line[cc]] += middle[Control_Line[cc] + 1];
			middle[Control_Line[cc]] += middle[Control_Line[cc] - 1];
			middle[Control_Line[cc]] /= 3;
		}
	}
}
void Ring_SingleLine_Right(void)
{
	float kright = lamada_outer_ring, kleft = lamada_inner_ring;

	// if (history_status[0] == Ring_step_three)
	// {
	// 	kleft = 1.15;
	// 	kright = 0.85;
	// }
	uint16 status[CAMERA_H][3];
	memset(status, 0, sizeof(status));

	int i, j;

	int len = 0;
	for (i = CAMERA_H - 2; i > 0; i--) // i = line,j = row
	{

		j = 0;
		//�ҵ���rp�߽�
		j = right[i]; //��ǰ��
		if (right[i] == -1)
			break;
		len = 0;
		int basic_j = j;
		j = basic_j;

		status[i][2] = j; //i����rp��

		while (img[i][j] == THRESHOLD && j >= 0)
		{
			j--;
		}

		status[i][1] = ++j; //i��zuo��

		len = status[i][2] - status[i][1];

		if (len >= road_len_min[i] && len <= road_len_max[i])
		{
			if (i < 30 && (status[i + 1][2]) >= 78 && (79 - status[i][2]) > 30 && (status[i + 2][2]) >= 78 && (status[i + 3][2]) >= 78)
			{
				status[i][0] = 0;
				break;
			}
			else if (
				(status[i][2] - status[i + 2][2]) > 1 &&
				(status[i + 2][0] != 0) && i < 56)
			{
				status[i][0] = 0;
			}
			else if (status[i][2] == 79)
			{
				status[i][0] = 0;
			}
			else
			{
				status[i][0] = len;
				middle[i] = (float)(status[i][1] * kleft + status[i][2] * kright) / 2;
			}
		}
		else
		{
			//("xxxxxx");
		}
	}
	int iuplast = 0, idownlast = 0;
	for (i = CAMERA_H - 1; i > 5; i--)
	{
		if (status[i][0] == 0)
		{
			int it = i, iup = i, idown = 0;
			while (status[it][0] == 0 && abs(it - i) < 40 && it > 4)
			{
				it--;
			} //������
			if (status[it][0] != 0)
			{
				iup = it;
				it = i;
				while (status[it][0] == 0 &&
					it < CAMERA_H - 2 &&
					abs(it - i) < 32)
				{
					it++;
				}
				if (status[it][0] != 0)
				{
					idown = it;
				}
				else
				{
					idown = 58;
					status[58][1] = 12;
					status[58][2] = 68;
				}
				int aver = 0;

				if (iup != iuplast && status[iup - 1][0] != 0 && status[iup - 2][0] != 0 && status[iup - 3][0] != 0)
				{
					aver = (status[iup][1] + status[iup - 1][1] + status[iup - 2][1] + status[iup - 3][1]) / 4;
					status[iup][1] = aver;
					status[iup - 1][1] = aver;
					status[iup - 2][1] = aver;
					status[iup - 3][1] = aver;
					aver = (status[iup][2] + status[iup - 1][2] + status[iup - 2][2] + status[iup - 3][2]) / 4;
					status[iup][2] = aver;
					status[iup - 1][2] = aver;
					status[iup - 2][2] = aver;
					status[iup - 3][2] = aver;
					iuplast = iup;
				}
				if (idown != idownlast && status[idown + 1][0] != 0 && status[idown + 2][0] != 0 && status[idown + 3][0] != 0)
				{
					aver = (status[idown][1] + status[idown + 1][1] + status[idown + 2][1] + status[idown + 3][1]) / 4;
					status[idown][1] = aver;
					status[idown + 1][1] = aver;
					status[idown + 2][1] = aver;
					status[idown + 3][1] = aver;
					aver = (status[idown][2] + status[idown + 1][2] + status[idown + 2][2] + status[idown + 3][2]) / 4;
					status[idown][2] = aver;
					status[idown + 1][2] = aver;
					status[idown + 2][2] = aver;
					status[idown + 3][2] = aver;
					idownlast = idown;
				}

				status[i][1] = status[iup][1] * (idown - i) + status[idown][1] * (i - iup);
				status[i][1] = division45((float)status[i][1] / (idown - iup));
				if (status[i][1] < 0)
					status[i][1] = 0;

				if ((status[i][2] == 79 || abs(right[i] - right[i - 1]) > 7) && i > 30)
				{
					if (!(i < 50 && abs(status[i + 1][2] - 79) <= 2 && abs(status[i + 2][2] - 79) <= 2))
					{
						status[i][2] = status[iup][2] * (idown - i) + status[idown][2] * (i - iup);
						status[i][2] = division45((float)status[i][2] / (idown - iup));
						if (status[i][2] > 79)
							status[i][2] = 79;
					}
				}
				int q1, q2;
				q1 = status[i][1];
				//[i][q1] = 200;

				q2 = status[i][2];
				//[i][q2] = 200;

				////////////////set center//////////////
				middle[i] = ((float)status[i][1] * kleft + (float)status[i][2] * kright) / 2;
			}
			else
			{ //�����Ҳ�����
				if (i > 18)
				{					  //���߹���
					int fix_flag = 0; //plan A
					for (it = i; it > 0; it--)
					{
						len = status[it][2] - status[it][1];
						if (len >= road_len_min[it] && len <= 32)
						{
							fix_flag = 1;
							status[it][0] = len;
							middle[it] = (status[it][1] + status[it][2]) / 2;
							break;
						}
					}

					if (fix_flag == 1 && abs(it - i) < 40 && it > 4)
					{
						i++;
						//("\nplanA�ſ��������ҵ�%d��", iup);

						//("\nplan A");
					}
					else
					{ //plan B,ִ�к��˳�
						//("\n  Attention!!  ");
						int cnt = 0;
						PosType data[30];
						memset(data, 0, sizeof(data));
						for (it = 58; it > 0; it--)
						{
							if (status[it][0] != 0)
							{

								data[cnt].x = it;
								data[cnt].y = status[it][0];
								cnt++;
								if (cnt >= 29)
									break;
							}
						}

						if (cnt > 15)
						{
							LeastSquareMethod(data, cnt);
							for (int ik = 58; right[ik] > 0 && ik > 4; ik--)
							{
								int tempper = right[ik] - k * ik - d;

								middle[ik] = (float)(tempper * kleft + right[ik] * kright) / 2;
							}
							//("\nplan B");
						}
						else
						{
							cnt = 0;
							for (it = 58; it > 0; it--)
							{
								if (status[it][1] > 0 && status[it][2] < 79 && status[it][2] > 40)
								{

									data[cnt].x = it;
									data[cnt].y = status[it][2] - status[it][1];
									cnt++;
									if (cnt >= 29)
										break;
								}
							}
							if (cnt > 15)
							{
								LeastSquareMethod(data, cnt);
								for (int ik = 58; right[ik] > 0 && ik > 4; ik--)
								{
									int tempper = right[ik] - k * ik - d;

									middle[ik] = (float)(tempper * kleft + right[ik] * kright) / 2;
								}
								//("\nplan B\n");
							}
							else
							{ //plan C,ʹ��Ĭ���������

								int whitecnt = 0;
								for (int kt = 0; kt < 80; kt++)
								{
									if (img[59][kt] == THRESHOLD)
										whitecnt++;
									if (img[58][kt] == THRESHOLD)
										whitecnt++;
									if (img[57][kt] == THRESHOLD)
										whitecnt++;
								}
								whitecnt /= 3;
								float k = (float)whitecnt / (float)road_len_max[59];

								for (it = 58; it > 5; it--)
								{
									if (right[it] >= 0)
									{

										int tempper = right[it] - k * road_len_max[it];
										middle[it] = (float)(tempper * kleft + right[it] * kright) / 2;
									}
								}
								//("\nplan C complete");
							}
						}

						//("alternative method executed,exit");
						return;
					}
				}
			}
			//	smooth_middle();
		}
	}
}
void Ring_SingleLine_Left(void)
{
	float kleft = lamada_outer_ring, kright = lamada_inner_ring;
	// if (history_status[0] == Ring_step_three)
	// {
	// 	kleft = 0.85;
	// 	kright = 1.15;
	// }
	uint16 status[CAMERA_H][3];
	memset(status, 0, sizeof(status));

	int i, j;

	int len = 0;
	for (i = CAMERA_H - 2; i > 0; i--) // i = line,j = row
	{

		j = 0;
		//�ҵ���߽�
		j = left[i]; //��ǰ��
		if (left[i] == -1)
			break;
		len = 0;
		int basic_j = j;
		j = basic_j;

		status[i][1] = j; //i�����

		while (img[i][j] == THRESHOLD && j < CAMERA_W)
		{
			j++;
		}

		status[i][2] = --j; //i���ұ�

		len = status[i][2] - status[i][1];

		//("\nline%d: %d <=    %d   <= %d", i, road_len_min[i], len, road_len_max[i]);

		if (len >= road_len_min[i] && len <= road_len_max[i])
		{
			if (i < 30 && (status[i + 1][1]) >= 1 && (status[i][1] - 0) > 30 && (status[i + 2][1]) >= 1 && (status[i + 3][1]) >= 1)
			{
				status[i][0] = 0;
				break;
			}
			else if (
				(status[i][2] - status[i + 2][2]) > 1 &&
				(status[i + 2][0] != 0) && i < 56)
			{
				status[i][0] = 0;
			}
			else if (status[i][2] == 79)
			{
				status[i][0] = 0;
			}
			else
			{
				status[i][0] = len;
				middle[i] = (float)(status[i][1] * kleft + status[i][2] * kright) / 2;
			}
		}
		else
		{
			//("xxxxxx");
		}
	}
	int iuplast = 0, idownlast = 0;
	for (i = CAMERA_H - 1; i > 5; i--)
	{
		if (status[i][0] == 0)
		{
			int it = i, iup = i, idown = 0;
			while (status[it][0] == 0 && abs(it - i) < 40 && it > 4)
			{
				it--;
			} //������
			if (status[it][0] != 0)
			{
				iup = it;
				it = i;
				while (status[it][0] == 0 &&
					it < CAMERA_H - 2 &&
					abs(it - i) < 32)
				{
					it++;
				}
				if (status[it][0] != 0)
				{
					idown = it;
				}
				else
				{
					idown = 58;
					status[58][1] = 12;
					status[58][2] = 68;
				}
				int aver = 0;

				if (iup != iuplast && status[iup - 1][0] != 0 && status[iup - 2][0] != 0 && status[iup - 3][0] != 0)
				{
					aver = (status[iup][1] + status[iup - 1][1] + status[iup - 2][1] + status[iup - 3][1]) / 4;
					status[iup][1] = aver;
					status[iup - 1][1] = aver;
					status[iup - 2][1] = aver;
					status[iup - 3][1] = aver;
					aver = (status[iup][2] + status[iup - 1][2] + status[iup - 2][2] + status[iup - 3][2]) / 4;
					status[iup][2] = aver;
					status[iup - 1][2] = aver;
					status[iup - 2][2] = aver;
					status[iup - 3][2] = aver;
					iuplast = iup;
				}
				if (idown != idownlast && status[idown + 1][0] != 0 && status[idown + 2][0] != 0 && status[idown + 3][0] != 0)
				{
					aver = (status[idown][1] + status[idown + 1][1] + status[idown + 2][1] + status[idown + 3][1]) / 4;
					status[idown][1] = aver;
					status[idown + 1][1] = aver;
					status[idown + 2][1] = aver;
					status[idown + 3][1] = aver;
					aver = (status[idown][2] + status[idown + 1][2] + status[idown + 2][2] + status[idown + 3][2]) / 4;
					status[idown][2] = aver;
					status[idown + 1][2] = aver;
					status[idown + 2][2] = aver;
					status[idown + 3][2] = aver;
					idownlast = idown;
				}

				if ((status[i][1] == 0 || abs(left[i] - left[i - 1]) > 7) && i > 30)
				{
					if (!(i < 50 && abs(status[i + 1][1] - 0) <= 2 && abs(status[i + 1][2] - 0) <= 2))
					{
						status[i][1] = status[iup][1] * (idown - i) + status[idown][1] * (i - iup);
						status[i][1] = division45((float)status[i][1] / (idown - iup));
						if (status[i][1] < 0)
							status[i][1] = 0;
					}
				}
				status[i][2] = status[iup][2] * (idown - i) + status[idown][2] * (i - iup);
				status[i][2] = division45((float)status[i][2] / (idown - iup));
				if (status[i][2] > 79)
					status[i][2] = 79;
				int q1, q2;
				q1 = status[i][1];
				q2 = status[i][2];

				////////////////set center//////////////
				middle[i] = ((float)status[i][1] * kleft + (float)status[i][2] * kright) / 2;
			}
			else
			{ //�����Ҳ�����
				if (i > 18)
				{					  //���߹���
					int fix_flag = 0; //plan A
					for (it = i; it > 0; it--)
					{
						len = status[it][2] - status[it][1];
						if (len >= road_len_min[it] && len <= 32)
						{
							fix_flag = 1;
							status[it][0] = len;
							middle[it] = (status[it][1] + status[it][2]) / 2;
							break;
						}
					}
					if (fix_flag == 1 && abs(it - i) < 40 && it > 4)
					{
						i++;
					}
					else
					{ //plan B,ִ�к��˳�
						int cnt = 0;
						PosType data[30];
						memset(data, 0, sizeof(data));
						for (it = 58; it > 0; it--)
						{
							if (status[it][0] != 0)
							{

								data[cnt].x = it;
								data[cnt].y = status[it][0];
								cnt++;

								if (cnt >= 29)
									break;
							}
						}

						if (cnt > 15)
						{
							LeastSquareMethod(data, cnt);
							for (int ik = 58; left[ik] >= 0 && ik > 4; ik--)
							{
								int temper = left[ik] + k * ik + d;
								middle[ik] = (float)(left[ik] * kleft + temper * kright) / 2;
							}
							//("\nplan B");
						}
						else
						{
							cnt = 0;
							for (it = 58; it > 0; it--)
							{
								if (status[it][1] > 0 && status[it][2] < 79 && status[it][2] > 40)
								{

									data[cnt].x = it;
									data[cnt].y = status[it][2] - status[it][1];
									cnt++;
									if (cnt >= 29)
										break;
								}
							}
							if (cnt > 15)
							{
								LeastSquareMethod(data, cnt);
								for (int ik = 58; left[ik] >= 0 && ik > 4; ik--)
								{
									int temper = left[ik] + k * ik + d;
									middle[ik] = (float)(left[ik] * kleft + temper * kright) / 2;
								}
							}
							else
							{

								int whitecnt = 0;
								for (int kt = 0; kt < 80; kt++)
								{
									if (img[59][kt] == THRESHOLD)
										whitecnt++;
									if (img[58][kt] == THRESHOLD)
										whitecnt++;
									if (img[57][kt] == THRESHOLD)
										whitecnt++;
								}
								whitecnt /= 3;
								float k = (float)whitecnt / (float)road_len_max[59];

								for (it = 58; it > 5; it--)
								{
									if (left[it] >= 0)
									{

										int temper = left[it] + k * road_len_max[it];
										middle[it] = (float)(left[it] * kleft + temper * kright) / 2;
									}
								}
							}
						}

						return;
					}
				}
			}
		}
	}
}
//line59--30  col 60--79
uint8 check_out_line_left_ring()
{
	for (int line = 30; line <= 59; line++)
	{
		if (img[line][60] == THRESHOLD && img[line][59] == THRESHOLD)
			;
		else
			return 0;
	}
	for (int col = 60; col < 80; col++)
	{
		if (img[32][col] == THRESHOLD && img[31][col] == THRESHOLD)
			;
		else
			return 0;
	}
	return 1;
}
//line59--30  col 0--19
uint8 check_out_line_right_ring()
{
	for (int line = 30; line <= 59; line++)
	{
		if (img[line][19] == THRESHOLD && img[line][20] == THRESHOLD)
			;
		else
			return 0;
	}
	for (int col = 0; col < 20; col++)
	{
		if (img[32][col] == THRESHOLD && img[31][col] == THRESHOLD)
			;
		else
			return 0;
	}
	return 1;
}
uint8 addtional_ring_checking_ring_polarity_left(uint8 addtional_ring_status)
{
	if (!check_out_line_left_ring())
		return addtional_ring_status;
	//�����ұ߰׿�
	int line = 0, col = 0;
	int white_cnt = 0, black_cnt = 0;

	for (line = 59; line > 30; line--)
	{
		for (col = 60; col < 80; col++)
		{
			if (img[line][col] == THRESHOLD)
				white_cnt++;
			else
				black_cnt++;
		}
	}														  //��ɼ���
	float k_loc = (float)black_cnt / (white_cnt + black_cnt); //��ɫ�ı���

	////("k_loc:%f", k_loc);
	if (addtional_ring_status == 0)
	{
		if (k_loc > 0.1)
			return addtional_ring_status + 1;
		else
			return addtional_ring_status;
	}
	else if (addtional_ring_status == 1) ///���½ǰ�ȫ
	{
		if (black_cnt <= 5)
			return addtional_ring_status + 1;
		else
			return addtional_ring_status;
	}
	else if (addtional_ring_status == 2)
	{
		if (k_loc > 0.1)
			return addtional_ring_status + 1;
		else
			return addtional_ring_status;
	}
	else if (addtional_ring_status == 3) //�ڶ���ȫ�� �ӽ�����
	{
		if (black_cnt <= 5)
			return addtional_ring_status + 1;
		else
			return addtional_ring_status;
	}
	else if (addtional_ring_status == 4)
		return addtional_ring_status;
}

uint8 addtional_ring_checking_ring_polarity_right(uint8 addtional_ring_status)
{
	if (!check_out_line_right_ring())
		return addtional_ring_status;
	//�����ұ߰׿�
	int line = 0, col = 0;
	int white_cnt = 0, black_cnt = 0;

	for (line = 59; line > 30; line--)
	{
		for (col = 0; col < 20; col++)
		{
			if (img[line][col] == THRESHOLD)
				white_cnt++;
			else
				black_cnt++;
		}
	}														  //��ɼ���
	float k_loc = (float)black_cnt / (white_cnt + black_cnt); //��ɫ�ı���

	////("k_loc:%f", k_loc);
	if (addtional_ring_status == 0)
	{
		if (k_loc > 0.1)
			return addtional_ring_status + 1;
		else
			return addtional_ring_status;
	}
	else if (addtional_ring_status == 1) ///���½ǰ�ȫ
	{
		if (black_cnt <= 5)
			return addtional_ring_status + 1;
		else
			return addtional_ring_status;
	}
	else if (addtional_ring_status == 2)
	{
		if (k_loc > 0.1)
			return addtional_ring_status + 1;
		else
			return addtional_ring_status;
	}
	else if (addtional_ring_status == 3) //�ڶ���ȫ�� �ӽ�����
	{
		if (black_cnt <= 5)
			return addtional_ring_status + 1;
		else
			return addtional_ring_status;
	}
	else if (addtional_ring_status == 4)
		return addtional_ring_status;
}

void Ring_Process(void)
{
	int RingDelatCntMax = 15;
	static float RingFindDistance = 0;
	//���ò��߲���//////////////////////////////////////////////////
	if (history_status[0] == Ring_step_one)
	{
		lamada_inner_ring = enter_ring;
		lamada_outer_ring = 2 - lamada_inner_ring;
	}
	else if (history_status[0] == Ring_step_two)
	{
		lamada_inner_ring = inside_ring;
		lamada_outer_ring = 2 - lamada_inner_ring;
	}
	else if (history_status[0] == Ring_step_three)
	{
		lamada_inner_ring = out_ring;
		lamada_outer_ring = 2 - lamada_inner_ring;
	}
	else
	{
		lamada_inner_ring = 1;
		lamada_outer_ring = 1;
	}
	//���ò��߲�������//////////////////////////////////////////////
	if (Ring_FIND == 1 && RingDelayCnt == 0)
	{
		RingDelayCnt = 1;
		RingFindDistance = Distance; //�״��ҵ����εľ���
	}
	if (RingDelayCnt > 0 && RingDelayCnt < RingDelatCntMax)
	{

		BELL_ON;
		RingDelayCnt++;
		int line;
		if (Ring_Exit_Deriction[Ring_cnt] == 0)
		{
			Ring_SingleLine_Right();
		}
		else
		{
			Ring_SingleLine_Left();
		}
	}
	else if (RingDelayCnt >= RingDelatCntMax)
	{

		if (Ring_Exit_Deriction[Ring_cnt] == 0)
		{
			Ring_SingleLine_Right();
		}
		else
		{
			Ring_SingleLine_Left();
		}

	}

	/*************************************************************/

	int Error =
		((int)(atan((middle[25] - MIDDLE) / (float)(60 - 25)) * 180 / 3.1415926));


	int direction_left = 1, direction_right = 0, ring_polarity = 0;

	ring_polarity = Ring_Exit_Deriction[Ring_cnt]; //������

	int *p = &Road_Error_Recording[9];
	for (int q = 0; q < 9; q++)
	{
		*p = *(p - 1);
		p--;
	}

	Road_Error_Recording[0] = Error;

	debug1 = 0;
	debug2 = 0;

	if (ring_polarity == direction_left)
	{ //��ת��
		addtional_ring_status = addtional_ring_checking_ring_polarity_left(addtional_ring_status);
		//Ϊ90��С��׼���Ķ����������
		////("addtional_ring_status%d", addtional_ring_status);
	}
	if (ring_polarity == direction_right)
	{ //youת��
		addtional_ring_status = addtional_ring_checking_ring_polarity_right(addtional_ring_status);
		//Ϊ90��С��׼���Ķ����������
		//("addtional_ring_status%d", addtional_ring_status);
	}

	if (current_ring_timing >= 2)
	{ //��error�ɼ�������ת��״̬
		//��ѯ�ϴ�״̬
		if (ring_polarity == direction_left)
		{											 //��ת��
			if (history_status[0] == Ring_step_zero) //��ʼ״̬
			{
				if (Road_Error_Recording[0] < 0 &&
					Road_Error_Recording[1] < 0 &&
					Road_Error_Recording[2] < 0) //�Ѿ�������ת״̬
				{
					current_status = Ring_step_one;
				}
				else
					current_status = Ring_step_zero; //����ά��ԭ��
			}
			else if (history_status[0] == Ring_step_one) //�Ѿ�������ת״̬
			{
				if (Road_Error_Recording[0] > +4 &&
					Road_Error_Recording[1] > 0 &&
					Road_Error_Recording[2] > 0)
				{
					current_status = Ring_step_two; //�Ѿ����뻷����ת״̬
				}
				else if (current_ring_timing >= 30 && addtional_ring_status == 4)
				{
					current_status = Ring_step_three;
					A_special_restriction_for_ring_out = 1;
				}
				else
					current_status = Ring_step_one; //����ά��ԭ��
			}
			else if (history_status[0] == Ring_step_two)
			{ //�Ѿ����뻷����ת״̬
				if (Road_Error_Recording[0] < -5 &&
					Road_Error_Recording[1] < 0 &&
					Road_Error_Recording[2] < 0)
				{
					current_status = Ring_step_three; //�Ѿ����������ת״̬
				}
				else
					current_status = Ring_step_two; //����ά��ԭ��
			}
			else if (history_status[0] == Ring_step_three)
			{
				if (current_ring_going_out_timing >= 3 && A_special_restriction_for_ring_out)
				{
					if (current_ring_timing >= 30 && A_special_restriction_for_ring_out >= 1 && right[58] < 75 && right[57] < 75 && right[56] < 75 && right[55] < 75 && right[50] < 75 && right[45] < 75 && right[40] < 75)
					{
						//��������������������������
						RingFindDistance = 0;
						Ring_cnt++;
						BELL_OFF;
						RingDelayCnt = 0;
						Ring_FIND = 0;
						current_status = Ring_just_out; //huigui
					}
					else
					{
						current_status = Ring_step_three; //ά��ԭ��
					}
				}
				else if (!A_special_restriction_for_ring_out)
				{
					int sum = 0, valid = 0;
					for (int line = 59; line > 40; line--)
					{
						for (int col = 79; col > 59; col--)
						{
							sum++;
							if (img[line][col] == THRESHOLD)
								valid++;
						}
					}
					float kk = (float)valid / sum;
					debug2 = kk * 100;
					//("kk%f  ", kk);
					if (kk > 0.75)
					{
						A_special_restriction_for_ring_out = 1;
						//	//("Restriction Removed");
					}
					//	//("%f", kk);
					current_status = Ring_step_three; //ά��ԭ��
				}
				else
				{
					current_status = Ring_step_three; //ά��ԭ��
				}
				current_ring_going_out_timing++;
			}
			else
			{
				current_status = Ring_step_zero;
			}
		}
		/**************��**************/
		if (ring_polarity == direction_right)
		{											 //youת��
			if (history_status[0] == Ring_step_zero) //��ʼ״̬
			{
				if (Road_Error_Recording[0] > 0 &&
					Road_Error_Recording[1] > 0 &&
					Road_Error_Recording[2] > 0) //�Ѿ�����youת״̬
				{
					current_status = Ring_step_one;
				}
				else
					current_status = Ring_step_zero; //����ά��ԭ��
			}
			else if (history_status[0] == Ring_step_one) //�Ѿ�����youת״̬
			{
				if (Road_Error_Recording[0] < -4 &&
					Road_Error_Recording[1] < 0 &&
					Road_Error_Recording[2] < 0)
				{
					current_status = Ring_step_two; //�Ѿ����뻷��zuoת״̬
				}
				else if (current_ring_timing >= 30 && addtional_ring_status == 4)
				{
					current_status = Ring_step_three;
					A_special_restriction_for_ring_out = 1;
				}
				else
					current_status = Ring_step_one; //����ά��ԭ��
			}
			else if (history_status[0] == Ring_step_two)
			{ //�Ѿ����뻷��zuoת״̬
				if (Road_Error_Recording[0] > +5 &&
					Road_Error_Recording[1] > 0 &&
					Road_Error_Recording[2] > 0)
				{
					current_status = Ring_step_three; //�Ѿ��������youת״̬
				}
				else
					current_status = Ring_step_two; //����ά��ԭ��
			}
			else if (history_status[0] == Ring_step_three)
			{ //�Ѿ����������ת״̬
				//�жϳ�����1.ʱ���������㣬2.�ұ��ҵ���û��
				if (current_ring_going_out_timing >= 3 && A_special_restriction_for_ring_out)
				{
					if (current_ring_timing >= 30 && A_special_restriction_for_ring_out >= 1 && left[58] > 4 && left[57] > 4 && left[56] > 4 && left[55] > 4 && left[50] > 4 && left[45] > 4 && left[40] > 4

						)
					{
						//��������������������������
						RingFindDistance = 0;
						Ring_cnt++;
						BELL_OFF;
						RingDelayCnt = 0;
						Ring_FIND = 0;
						current_status = Ring_just_out;
					}
					else
					{
						current_status = Ring_step_three; //ά��ԭ��
					}
				}
				else if (!A_special_restriction_for_ring_out)
				{
					int sum = 0, valid = 0;
					for (int line = 59; line > 40; line--)
					{
						for (int col = 0; col < 20; col++)
						{
							sum++;
							if (img[line][col] == THRESHOLD)
								valid++;
						}
					}
					float kk = (float)valid / sum;
					if (kk > 0.75)
						A_special_restriction_for_ring_out = 1;
					//		//("%f", kk);
					current_status = Ring_step_three; //ά��ԭ��
				}
				else
				{
					current_status = Ring_step_three; //ά��ԭ��
				}
				current_ring_going_out_timing++;
			}
			else
			{
				current_status = Ring_step_zero;
			}
		}
	}
	else
	{
		current_status = Ring_step_zero;
	}
	current_ring_timing++;
}

void MiddleLine_On(void)
{
	int line, column;
	for (line = RowMax; line >= RowMin; line--)
	{
		if (left[line] != -1 || line > 25)
		{
			if (middle[line] >= 0 && middle[line] < 80)
			{

				img[line][middle[line]] = 0;
			}
		}
	}
}

uint8 Is_A_Hill()
{
	int find_perfect_straight_road_in_history = 0;
	for (int p = 0; p < 30; p++)
	{ //��ѯʶ����ʷ�����ǲ����б�׼��ֱ��
		if (history_status[p] == Straight_Perfect)
			find_perfect_straight_road_in_history = 1;
	}
	if (find_perfect_straight_road_in_history == 0)
		return 0;

	int len = 0;
	int delta = 0;
	int score = 0;
	for (int line = 18; line <= 30; line++)
	{
		len = right[line] - left[line];
		delta = (len - (road_len_max[line] - 8));
		if (delta > 10)
			score += 3;
		else if (delta > 7)
			score += 1;
		else if (delta < 0)
			score -= 10;
	}
	if (score > 18)
		return 1;
	else
		return 0;
}

uint8 hill_road_len[60] = {
	29, 29, 29, 29, 29, 29, 30, 31, 31, 32,
	32, 34, 34, 34, 35, 36, 36, 37, 37,
	38, 39, 39, 40, 41, 41, 42, 42, 42,
	44, 44, 44, 45, 46, 46, 46, 47, 47,
	48, 49, 49, 49, 50, 51, 51, 51, 53,
	53, 53, 54, 55, 56, 56, 56, 57, 58,
	59, 59, 59, 61, 61 };

uint8 check_hill_character()
{
	int hill = 1;
	int score = 0;

	for (int line = RowMax; line > 13; line--)
	{
		int len = right[line] - left[line];
		if (len <= 10)
			score -= 50;
		score += len - hill_road_len[line];
	}
	////("\nscore:%d", score);
	if (score > 100)
		return 1;

	return 0;
}
uint8 StraightRoad_Detection()
{
	int min_line_with_middle = 60;
	int line;
	for (line = RowMax; line > 0; line--)
	{
		if (middle[line] != 0)
			min_line_with_middle = line;
		else
			break;
	}
	if (line > 10)
		return 0;

	PosType middleP[42];
	int cnt = 0;
	for (line = RowMax; line > 40; line--)
	{
		if (left[line] <= 0 || right[line] >= 79 || right[line] <= 0)
			return 0;
		middleP[cnt].x = line;
		middleP[cnt].y = middle[line];
		cnt++;
	}
	LeastSquareMethod(middleP, cnt);
	float k_local = k;
	float d_local = d;
	if (fabs(k_local) > 0.15)
		return 0; //��̫б���˳�

	for (line = RowMax; line > 13; line--)
	{

		int j = line * k_local + d_local;
		if (
			abs(j - middle[line]) <= 3)
		{
			continue;
		}
		else
		{

			return 0;
		}
	}
	//("\nֱ����");
	return 1;
}

uint8 check_long_straight_line()
{
	int cnt = 0;
	int straight_cnt = 0;
	if (
		history_status[0] != Straight_Perfect && history_status[0] != Straight_Line_Very_Long && history_status[1] != Straight_Perfect && history_status[1] != Straight_Line_Very_Long && history_status[2] != Straight_Perfect && history_status[2] != Straight_Line_Very_Long)
		return 0;

	for (int p = 0; p < 25; p++)
	{
		cnt++;
		if (history_status[p] == Straight_Perfect || history_status[p] == Straight_Line_Very_Long)
			straight_cnt++;
	}
	float kk = (float)straight_cnt / cnt;
	if (kk > 0.8)
		return 1;
	else
		return 0;
}

uint8 decide_barrier_polarity()
{
	int col = 0;
	uint8 col1 = 0, col2 = 0;
	for (int row = RowMax; row > 7; row -= 2)//�ӵײ���ͷ��
	{
		for (col = left[row]; col <= right[row]; col++)//������ص��ұ���
		{
			if (img[row][col] == 0)//���ֺڵ�
			{
				col1 = col;//�ϰ������
				for (; img[row][col] == 0 && col <= right[row]; col++)//ֱ�����ϰ����ұ�
					;
				col2 = col;//�������ұ�
				col = (col1 + col2) / 2;//�ϰ�������
				if (col < middle[row])//�ϰ����������������
				{
					return left_barrier;//�����
				}
				else
				{
					return right_barrier;//���ұ�
				}
			}
		}
	}

	return 0;
}

uint8 barrier_detaction_left()
{
	uint8 leftx[60];
	uint8 len[60];
	memset(leftx, 0, sizeof(leftx));
	memset(len, 0, sizeof(len));

	int col = 0;
	for (int row = RowMax; row > 5; row--)
	{
		if (right[row] <= 79 && right[row] > 20)
		{
			for (col = right[row]; col > left[row] && img[row][col] == THRESHOLD; col--)
				;
			if (img[row][col] == THRESHOLD)
			{
				leftx[row] = col;
				len[row] = right[row] - leftx[row];
			}
			else if (img[row][col + 1] == THRESHOLD)
			{
				leftx[row] = col + 1;
				len[row] = right[row] - leftx[row];
			}
			else
			{
			}
		}
	}

	uint8 suoxiaodian = 0;
	uint8 pengzhangdian = 0;
	uint8 barrier_rows = 0;
	for (int row = RowMax; row > 5; row--)
	{
		if (len[row] > 0 && len[row - 1] > 0 && len[row - 2] > 0)
		{
			if (suoxiaodian == 0)
			{

				if (

					(row < 40 && ((len[row] - len[row - 1]) >= 6 || (len[row] - len[row - 2]) >= 6))

					||

					(row >= 40 && ((len[row] - len[row - 1]) >= 6 || (len[row] - len[row - 2]) >= 8))

					)
				{

					if (leftx[row] > 2)
					{
						/////////////more check///////////////
						int16 len_sum_up_before_barrier = 0;
						int16 len_sum_up_in_barrier = 0;

						len_sum_up_in_barrier = 0;
						len_sum_up_before_barrier = 0;
						for (uint16 sub_row = row - 1 + 5; sub_row > row - 1; sub_row--) //������
							len_sum_up_before_barrier += len[sub_row];
						for (uint16 sub_row = row - 1 - 5; sub_row < row - 1; sub_row++) //������
							len_sum_up_in_barrier += len[sub_row];

						if (len_sum_up_before_barrier != 0)
						{

							float bi = (float)len_sum_up_in_barrier / len_sum_up_before_barrier;
							if (row <= 50 && row > 25 && (bi > 0.75 || bi < 0.3)) //δͨ����һ�����
							{
								return 0;
							}
						}

						////////////////////////////////////
						suoxiaodian = row;

						if (row < 13)
							return 0;
					}
				}
			}
			else if (pengzhangdian == 0)
			{
				barrier_rows++;
				if (suoxiaodian - row >= 3 &&
					((len[row - 1] - len[row]) >= 2 || (len[row - 2] - len[row]) >= 2))
				{
					pengzhangdian = row;
				}
			}
		}
	}

	if (barrier_rows >= 5 && barrier_rows < 30 && pengzhangdian > 5)
	{
		return 1;
	}
	else
		return 0;
}
uint8 barrier_detaction_right()
{
	uint8 rightx[60];
	uint8 len[60];
	memset(rightx, 0, sizeof(rightx));
	memset(len, 0, sizeof(len));

	int col = 0;
	for (int row = RowMax; row > 5; row--)
	{
		if (left[row] > 0 && left[row] < 60)
		{
			//xianzhi
			for (col = left[row]; col < right[row] && img[row][col] == THRESHOLD; col++)
				;
			if (img[row][col] == THRESHOLD)
			{
				rightx[row] = col;
				len[row] = rightx[row] - left[row];
			}
			else if (img[row][col - 1] == THRESHOLD)
			{
				rightx[row] = col - 1;
				len[row] = rightx[row] - left[row];
			}
			else
			{
			}
		}
	}

	uint8 suoxiaodian = 0;
	uint8 pengzhangdian = 0;
	uint8 barrier_rows = 0;
	for (int row = RowMax; row > 5; row--)
	{
		if (len[row] > 0 && len[row - 1] > 0 && len[row - 2] > 0)
		{
			if (suoxiaodian == 0)
			{

				if (

					(row < 40 && ((len[row] - len[row - 1]) >= 6 || (len[row] - len[row - 2]) >= 6))

					||

					(row >= 40 && ((len[row] - len[row - 1]) >= 6 || (len[row] - len[row - 2]) >= 8))

					)
				{

					if (rightx[row] < 77) //��ֹʮ������
					{
						/////////////more check///////////////
						int16 len_sum_up_before_barrier = 0;
						int16 len_sum_up_in_barrier = 0;

						len_sum_up_in_barrier = 0;
						len_sum_up_before_barrier = 0;
						for (uint16 sub_row = row - 1 + 5; sub_row > row - 1; sub_row--) //������
							len_sum_up_before_barrier += len[sub_row];
						for (uint16 sub_row = row - 1 - 5; sub_row < row - 1; sub_row++) //������
							len_sum_up_in_barrier += len[sub_row];

						if (len_sum_up_before_barrier != 0)
						{

							float bi = (float)len_sum_up_in_barrier / len_sum_up_before_barrier;
							if (row <= 50 && row > 25 && (bi > 0.75 || bi < 0.3)) //δͨ����һ�����
							{
								return 0;
							}
						}

						////////////////////////////////////
						suoxiaodian = row;

						if (row < 13)
							return 0;
					}
				}
			}
			else if (pengzhangdian == 0)
			{
				barrier_rows++;
				if (suoxiaodian - row >= 3 &&
					((len[row - 1] - len[row]) >= 2 || (len[row - 2] - len[row]) >= 2))
				{
					pengzhangdian = row;
				}
			}
		}
	}

	if (barrier_rows >= 5 && barrier_rows < 30 && pengzhangdian > 5)
	{
		return 1;
	}
	else
		return 0;
}

uint8 Is_left_line_straight()
{
	int min_line_with_left = 60;
	int line;
	for (line = RowMax; line > 15; line--)
	{
		if (left[line] > 5 && left[line] < 50 && abs(left[line] - left[line - 1]) <= 3)
			min_line_with_left = line;
		else
			break;
	}
	if (line > 18)
	{
		//("\n ���û��ͨ��ֱ����⣡");
		return 0;
	}

	PosType leftP[42];
	int cnt = 0;
	for (line = RowMax; line > 40; line--)
	{
		leftP[cnt].x = line;
		leftP[cnt].y = left[line];
		cnt++;
	}
	LeastSquareMethod(leftP, cnt);
	float k_local = k;
	float d_local = d;

	for (line = RowMax; line > 13; line--)
	{

		int j = line * k_local + d_local;
		if (
			abs(j - left[line]) <= 4)
		{
			continue;
		}
		else
		{
			//("\n ���û��ͨ��ֱ����⣡");
			return 0;
		}
	}
	//("\n ���ͨ��ֱ����⣡");
	return 1;
}

uint8 Is_right_line_straight()
{
	int min_line_with_right = 60;
	int line;
	for (line = RowMax; line > 15; line--)
	{
		if (right[line] < 79 && right[line] > 30 && abs(right[line] - right[line - 1]) <= 3)
			min_line_with_right = line;
		else
			break;
	}
	if (line > 18)
	{
		//("\n �ұ�û��ͨ��ֱ����⣡");
		return 0;
	}

	PosType rightP[42];
	int cnt = 0;
	for (line = RowMax; line > 40; line--)
	{
		rightP[cnt].x = line;
		rightP[cnt].y = right[line];
		cnt++;
	}
	LeastSquareMethod(rightP, cnt);
	float k_local = k;
	float d_local = d;

	for (line = RowMax; line > 13; line--)
	{

		int j = line * k_local + d_local;
		if (
			abs(j - right[line]) <= 4)
		{
			continue;
		}
		else
		{
			//("\n �ұ�û��ͨ��ֱ����⣡");
			return 0;
		}
	}
	//("\n �ұ�ͨ��ֱ����⣡");
	return 1;
}

uint8 barrier_action_delayer = 0;//�����ϰ��Ĵ���

uint8 check_barrier_status()
{
	uint8 leftx[60];
	memset(leftx, 0, sizeof(leftx));

	uint8 rightx[60];
	uint8 len[60];
	memset(rightx, 0, sizeof(rightx));
	memset(len, 0, sizeof(len));
	int col = 0;
	if (Barrier_existance_and_polarity == left_barrier)
	{
		for (int row = RowMax; row > 5; row--)
		{
			if (right[row] < 75 && right[row] > 20)
			{
				//xianzhi
				for (col = right[row]; col > left[row] && img[row][col] == THRESHOLD; col--)
					;
				if (img[row][col] == THRESHOLD)
				{
					leftx[row] = col;
					len[row] = right[row] - leftx[row];
				}
				else if (img[row][col + 1] == THRESHOLD)
				{
					leftx[row] = col + 1;
					len[row] = right[row] - leftx[row];
				}
				else
				{
				}
			}
		}
	}
	else if (Barrier_existance_and_polarity == right_barrier)
	{

		for (int row = RowMax; row > 5; row--)
		{
			if (left[row] > 4 && left[row] < 60)
			{
				//xianzhi
				for (col = left[row]; col < right[row] && img[row][col] == THRESHOLD; col++)
					;
				if (img[row][col] == THRESHOLD)
				{
					rightx[row] = col;
					len[row] = rightx[row] - left[row];
				}
				else if (img[row][col - 1] == THRESHOLD)
				{
					rightx[row] = col - 1;
					len[row] = rightx[row] - left[row];
				}
				else
				{
				}
			}
		}
	}

	if (barrier_status == ba_phase_one)
	{
		barrier_action_delayer = 0;
		if (
			len[RowMax] < 43 ||
			len[RowMax - 1] < 40 ||
			len[RowMax - 3] < 37

			)
			barrier_status = ba_phase_two;
	}
	else if (barrier_status == ba_phase_two)
	{
		barrier_action_delayer++;

		if (
			len[RowMax] > 45 &&
			len[RowMax - 1] > 45 &&
			len[RowMax - 5] > 45 &&
			len[RowMax - 9] > 45)
			barrier_status = ba_phase_three;
	}
	else if (barrier_status == ba_phase_three)
	{
		barrier_status = ba_phase_four;
	}
	else if (barrier_status == ba_phase_four)
	{
		return 1;
	}
	return 0;
}

uint8 Is_left_line_straight_evo()
{
	int min_line_with_left = 60;
	int line_start_delayer = RowMax;
	int line;
	for (line = RowMax; line > 15; line--)
	{
		if (left[line] >= 3 && left[line] < 50 && abs(left[line] - left[line - 1]) <= 3)
			min_line_with_left = line;
		else if (left[line] >= 0 && line >= 50 && abs(left[line] - left[line - 1]) <= 3)
		{
			min_line_with_left = line;
			line_start_delayer = line; //��ʼ�۲�������Ӻ�
		}							   //��û�б��line_start_delayer����ֵΪRowMax
		else
			break;
	}
	if (line > 18 || line_start_delayer < 50)
	{
		return 0;
	}

	PosType leftP[42];
	int cnt = 0;
	for (line = line_start_delayer; line > (35 - (RowMax - line_start_delayer)); line--) ///���������Ҳ��֮�Ӻ�
	{
		leftP[cnt].x = line;
		leftP[cnt].y = left[line];
		cnt++;
	}
	LeastSquareMethod(leftP, cnt);
	float k_local = k;
	float d_local = d;

	for (line = line_start_delayer; line > 13; line--)
	{

		int j = line * k_local + d_local;
		if (
			abs(j - left[line]) <= 4)
		{
			continue;
		}
		else
		{
			return 0;
		}
	}
	return 1;
}
uint8 Is_right_line_straight_evo()
{
	int min_line_with_right = 60;
	int line_start_delayer = RowMax;
	int line;
	for (line = RowMax; line > 15; line--)
	{
		if (right[line] < 77 && right[line] > 30 && abs(right[line] - right[line - 1]) <= 3)
			min_line_with_right = line;
		else if (right[line] <= 79 && line >= 50 && abs(right[line] - right[line - 1]) <= 3)
		{
			min_line_with_right = line;
			line_start_delayer = line; //��ʼ�۲�������Ӻ�
		}							   //��û�б��line_start_delayer����ֵΪRowMax
		else
			break;
	}
	if (line > 18 || line_start_delayer < 50)
	{
		return 0;
	}

	PosType rightP[42];
	int cnt = 0;
	for (line = line_start_delayer; line > (35 - (RowMax - line_start_delayer)); line--)
	{
		rightP[cnt].x = line;
		rightP[cnt].y = right[line];
		cnt++;
	}
	LeastSquareMethod(rightP, cnt);
	float k_local = k;
	float d_local = d;

	for (line = line_start_delayer; line > 13; line--)
	{

		int j = line * k_local + d_local;
		if (
			abs(j - right[line]) <= 4

			)
		{
			continue;
		}
		else
		{
			//("\n �ұ�û��ͨ��ֱ����⣡");
			return 0;
		}
	}
	//("\n �ұ�ͨ��ֱ����⣡");
	return 1;
}

void barrier_status_transform()//�ϰ����ж�
{
	int temp_result = decide_barrier_polarity();//�ж��ϰ�������໹���Ҳ�
	//("\ntemp_result%d", temp_result);
	if (temp_result == right_barrier)//���Ҳ�
	{
		Barrier_existance_and_polarity = 1;
		barrier_status = ba_phase_one;//�������ϰ�
		BELL_ON;
		current_status = Straight_Perfect;//��ǰ״̬������ֱ��
		barrier_start_distance = Distance;//
		barrier_action_delayer = 0;
		gpio_set(LED_RED, 1);
	}
	if (temp_result == left_barrier)
	{
		Barrier_existance_and_polarity = 2;
		barrier_status = ba_phase_one;
		BELL_ON;
		current_status = Straight_Perfect;
		barrier_start_distance = Distance;//Distance��ʼ��Ϊ0��
		barrier_action_delayer = 0;//
		gpio_set(LED_RED, 1);
	}
}

uint8 check_barrier_status_evo()
{
	uint8 leftx[60];
	memset(leftx, 0, sizeof(leftx));

	uint8 rightx[60];
	uint8 len[60];

	memset(rightx, 0, sizeof(rightx));
	memset(len, 0, sizeof(len));
	int col = 0;
	if (Barrier_existance_and_polarity == left_barrier)
	{
		for (int row = RowMax; row > 5; row--)
		{
			if (right[row] <= 79 && right[row] > 20)
			{
				//xianzhi
				for (col = right[row]; col > left[row] && img[row][col] == THRESHOLD; col--)
					;
				if (img[row][col] == THRESHOLD)
				{
					leftx[row] = col;
					len[row] = right[row] - leftx[row];
				}
				else if (img[row][col + 1] == THRESHOLD)
				{
					leftx[row] = col + 1;
					len[row] = right[row] - leftx[row];
				}
				else
				{
				}
			}
		}
	}
	else if (Barrier_existance_and_polarity == right_barrier)
	{

		for (int row = RowMax; row > 5; row--)
		{
			if (left[row] >= 0 && left[row] < 60)
			{
				//xianzhi
				for (col = left[row]; col < right[row] && img[row][col] == THRESHOLD; col++)
					;
				if (img[row][col] == THRESHOLD)
				{
					rightx[row] = col;
					len[row] = rightx[row] - left[row];
				}
				else if (img[row][col - 1] == THRESHOLD)
				{
					rightx[row] = col - 1;
					len[row] = rightx[row] - left[row];
				}
				else
				{
				}
			}
		}
	}

	//////////////////////////////////////////new

	uint8 jump_line = 0, temp_j = 0, evidence = 0;

	int percise_intering = 0; //��׼

	for (uint8 row = 35; row > 22; row--)
	{
		if ((len[row] - len[row + 1]) >= 8 && len[row] > 0)
			jump_line = row;
	}

	if (jump_line != 0 && barrier_status == ba_phase_one)
	{

		if (Barrier_existance_and_polarity == left_barrier)
		{
			for (uint8 row = RowMax; row > (jump_line + 1); row--)
			{
				if (abs(leftx[row] - leftx[row - 1]) >= 2) //��ֹ����
				{
					break;
					percise_intering = -1;
				}
				temp_j = 0;
				while (img[row][temp_j] == 0 && temp_j <= leftx[row])
					temp_j++; //�ҵ����е�һ����
				if ((leftx[row] - temp_j) >= 10 && (leftx[row] - temp_j) <= 20)
				{
					evidence++;
				}
			}
		}
		if (Barrier_existance_and_polarity == right_barrier)
		{
			for (uint8 row = RowMax; row > (jump_line + 1); row--)
			{
				if (abs(rightx[row] - rightx[row - 1]) >= 2) //��ֹ����
				{
					break;
					percise_intering = -1;
				}
				temp_j = 79;
				while (img[row][temp_j] == 0 && temp_j >= rightx[row])
					temp_j--; //�ҵ����е�һ����
				if ((temp_j - rightx[row]) >= 10 && (temp_j - rightx[row]) <= 20)
				{
					evidence++;
				}
			}
		}

		if (percise_intering != -1)
		{
			if (evidence > 10)
				percise_intering = 1;
		}
	}

	if (barrier_status == ba_phase_one)
	{
		barrier_action_delayer = 0;

		if (
			(
			(len[RowMax] < 45 && len[RowMax] != 0) +
				(len[RowMax - 1] < 45 && len[RowMax - 1] != 0) +
				(len[RowMax - 2] < 45 && len[RowMax - 2] != 0) +
				(len[RowMax - 3] < 45 && len[RowMax - 3] != 0) +
				(len[RowMax - 4] < 45 && len[RowMax - 4] != 0)

				) >= 3 ||
			percise_intering == 1)

			barrier_status = ba_phase_two;
	}

	else if (barrier_status == ba_phase_two)
	{
		barrier_action_delayer++;

		if (
			len[RowMax] > 45 &&
			len[RowMax - 1] > 45 &&
			len[RowMax - 5] > 45 &&
			len[RowMax - 9] > 45)
			barrier_status = ba_phase_three;
	}
	else if (barrier_status == ba_phase_three)
	{
		barrier_status = ba_phase_four;
	}
	else if (barrier_status == ba_phase_four)
	{
		return 1;
	}

	return 0;
}

void clean_shadow()
{
	/*ͼ��ײ������ɫ���ֽ϶࣬��ͼ�� �ײ�ȫ��ˢ�ף�����������������Ӱ�������ɫ���ֽ��٣�����Ϊ�����ĳ������Ԫ����ͼ��ײ�*/
	for (int col = 25; col < 55; col++)//25�е�55��
	{
		int evidence = 0;
		if (img[59][col] == 0 || img[58][col] == 0 || img[57][col] == 0 || img[56][col] == 0 || img[55][col] == 0)
		{

			for (int row = 54; row > 44; row--)
			{
				if (img[row][col] == THRESHOLD)
				{
					evidence++;
				}
				else
				{
					evidence = 0;
					break;
				}
			}
			if (evidence >= 9)
			{
				img[59][col] = THRESHOLD;
				img[58][col] = THRESHOLD;
				img[57][col] = THRESHOLD;
				img[56][col] = THRESHOLD;
				img[55][col] = THRESHOLD;
			}
		}
	}
}

void MiddleLine_GridOn(void)
{
	int line, column;
	for (line = RowMax; line >= RowMin; line--)
	{
		if (left[line] != -1)
		{
			img[line][middle[line]] = 0;
		}
		else
			break;
		img[line][39] = 0;
		//		img[line][29] = 0;
		//		img[line][49] = 0;
		//		img[line][19] = 0;
		//		img[line][9] = 0;
		//		img[line][59] = 0;
		//		img[line][69] = 0;
	}
	for (column = 0; column <= 79; column++)
	{
		//		img[19][column] = 0;
		//		img[29][column] = 0;
		img[39][column] = 0;
		//		img[49][column] = 0;
		//		img[59][column] = 0;
	}
}
uint8 Stepping_over_zebra()
{
	int i = 0;
	int j = 0;
	uint8 evidence = 0;
	head_clear_flag = 1;//

	for (i = 45; i <= 59; ++i)
	{
		evidence = 0;
		uint8 *pimg = &img[i][0];
		for (j = 15; j < 65; j++)
		{
			if (*(pimg + j) != *(pimg + j + 1))//���ڵ��������Ȳ���ȣ��о�+1
				evidence++;
		}
		if (evidence >= 5 && i >= 53)//����Ҳ໹��
			head_clear_flag = 0;
		if (evidence > 12)//6�������ߣ��ж�Ϊ������
			return 1;
	}
	return 0;
}
uint8 Super_Ring_Check(const float k_local_left, const float d_local_left, const float k_local_right, const float d_local_right)
{
	uint8 have_all_black = 0;
	int cnt_tempp = 0, cnt;
	uint8 line, col, step = 0;

	uint8 record[8];
	memset(record, 0, sizeof(record));

	uint8 end_center = (d_local_left + k_local_left * 0 + d_local_right + k_local_right * 0) / 2;

	if (end_center < 12 || end_center > 68)
		return 0;

	for (col = 0; col <= 79; ++col)
	{
		step = 0;
		line = 0;
		while (step <= 5 && img[line++][col] == 0)
			++step;
		if (col >= end_center - 20 && col <= end_center + 20 && step <= 0)
			return 0;
		record[step]++;
	}
	if (record[1] < 10 && (record[1] + record[2]) < 18)
		;
	else
		return 0;

	uint8 line_all_black = 3, all_black_cnt = 0, line_all_white_end = 0, line_all_white = 0, res, status = 1, middle_status_cnt = 0, all_white_cnt = 0;

	uint16 leftb, rightb;

	for (line = 0; line <= 38; ++line)
	{

		leftb = d_local_left + k_local_left * line;
		rightb = d_local_right + k_local_right * line;
		/********************************************************************/
		if (status == 1 && all_black_cnt++ < 15) //��ʱ�ڴ�
		{

			cnt = rightb - leftb;
			if (line > 10 && cnt < 10)
			{
				break;
			}
			else if (line <= 10 && cnt < 5)
				break;
			int cnt_confirm = 0;
			for (int col = leftb; col <= rightb; col++)
			{
				if (img[line][col] == 0)
					cnt_confirm++;
			}

			//�м��Ƿ�ȫ��
			if (abs(cnt_confirm - cnt) <= 2) //�м�ȫ�Ǻ�
			{
				while (img[line][leftb--] == 0 && leftb > 0)
				{
					step++;
				}
				if (step < 10)
				{
					status = 2; //������
					line_all_black = line;
				}

				step = 0;
				while (img[line][rightb++] == 0 && rightb < 79)
				{
					step++;
				}
				if (step < 10)
				{
					status = 2; //������
					line_all_black = line;
				}
			}
			else
			{
				line_all_black = line;
				//middle_status_cnt = 0;
				status = 2;
			}
		}
		/********************************************************************/
		if (status == 2) //��ʱ����
		{
			res = Is_line_all_white_or_black(line);
			if (res == 0 || res == 2)
				middle_status_cnt++; //�ӻ��
			else if (res == 1)
			{
				status = 3;
			}

			if (middle_status_cnt >= 5)
			{
				//("����̫��");
				return 0;
			}
		}
		if (status == 3) //��ʱȫ��
		{
			res = Is_line_all_white_or_black(line);
			if (res == 1)
				all_white_cnt++;
			else
			{
				status = 0;
				if (all_white_cnt >= 6)
					return 1;
				else
				{
					//("����̫��%d", all_white_cnt);
					return 0;
				}
			}
			if (all_white_cnt >= 14)
				return 1;
		}
	}

	//("��β�˳�");
	return 0;
}

int Ring_Curnor_Line[2] = { 0 };//Բ��������

void check_cross_result()
{
	uint8 corner_line = Ring_Curnor_Line[0];
	if (Ring_Curnor_Line[1] > Ring_Curnor_Line[0])
		corner_line = Ring_Curnor_Line[1];

	if (corner_line > 40 || corner_line < 19)
		return; //��Ч��Ϣ̫���޷�����
	//������

	uint8 line, col, cnt;
	PosType middleP[60];

	for (line = RowMax, cnt = 0; line > corner_line; line--)
	{
		middleP[cnt].x = line;
		middleP[cnt].y = middle[line];
		cnt++;
	}
	LeastSquareMethod(middleP, cnt);
	float k_local_middle = k;
	float d_local_middle = d;

	uint8 m_col, alert = 0;
	for (line = RowMax; line > 20; line--)
	{
		m_col = middle[line];
		col = k_local_middle * line + d_local_middle;

		if (abs(col - m_col) > 20)
		{
			if ((col < 35 && m_col > 45) || (col > 45 && m_col < 35))
			{

				alert = 1;
				//		break;
			}
		}
	}
}

void Ring_Detection(void) //����·�ڼ��
{
	int Ring_Confirm_first = 0, Ring_Confirm_second = 0, Ring_Confirm_third = 0, Ring_Confirm_forth = 0;
	short line, column;
	int Ring_Curnor[2] = { 0 };//��ࡢ�Ҳ���Բ��������
	Ring_Curnor_Line[0] = 0;
	Ring_Curnor_Line[1] = 0;
	//��ʾ����Բ��ʱ������ȵ��й��ɱ��;���Բ���н�
	for (line = RowMax - 5; line >= RowMin + 5; line--)
	{
		int width[2][3] = { 0 };
		int i = 0;
		for (i = 0; i < 3; i++)//width[0]�洢���Ե�����ĵĿ��
		{
			if (left[line - i] > 0 && right[line - i] < 79)
			{
				width[0][i] = 39 - left[line - i];
				width[1][i] = right[line - i] - 39;//width[1]�洢�ұ�Ե�����ĵĿ��
			}
			else
				break;
		}
		if (width[0][2] > width[0][1] && width[0][1] > width[0][0] && width[1][2] > width[1][1] && width[1][1] > width[1][0])
		{
			Ring_Confirm_first = 1;
			break;
		}
		else if (width[0][2] > width[0][1] && width[0][1] >= width[0][0] && width[1][2] > width[1][1] && width[1][1] > width[1][0])
		{
			Ring_Confirm_first = 1;
			break;
		}
		else if (width[0][2] > width[0][1] && width[0][1] > width[0][0] && width[1][2] > width[1][1] && width[1][1] >= width[1][0])
		{
			Ring_Confirm_first = 1;
			break;
		}
		else if (width[0][2] >= width[0][1] && width[0][1] >= width[0][0] && width[1][2] > width[1][1] && width[1][1] >= width[1][0])
		{
			Ring_Confirm_first = 1;
			break;
		}
	}

	//�����սǣ�����������Ϊ����
	for (line = RowMax - 5; line > RowMin + 10; line--)
	{
		if (left[line] > left[line + 5] && (left[line] - 5) > left[line - 5])//��ǰ���Ե�����������5�е�������
		{
			if (left[line - 2] > 0 && left[line - 1] > 0)
			{
				Ring_Curnor_Line[0] = line;//Բ��������
				Ring_Curnor[0] = 1;//�����Բ
				break;
			}
		}
	}
	for (line = RowMax - 5; line > RowMin + 10; line--)//��������Ҳ�Ϊ����
	{
		if (right[line] < right[line + 5] && (right[line] + 5) < right[line - 5])
		{
			if (right[line - 2] < 79 && right[line - 1] < 79)
			{
				Ring_Curnor_Line[1] = line;
				Ring_Curnor[1] = 1;
				break;
			}
		}
	}
	if (Ring_Curnor[0] == 1 && Ring_Curnor[1] == 1 && (Ring_Curnor_Line[0] - Ring_Curnor_Line[1]) > -5 && (Ring_Curnor_Line[0] - Ring_Curnor_Line[1]) < 5)
		Ring_Confirm_second = 1;//Ring_Curnor  ����ͬʱ��Բ���ж���������
	//��ֱ�����
	float ZHI_NUM = 20.0;
	float temp_k_more[2][3] = { 0 };
	float temp_k[2] = { 0 };
	float temp_err[2] = { 0 };
	temp_k[0] = float(left[RowMax - (int)ZHI_NUM] - left[RowMax]) / ZHI_NUM;//���Ե�ײ������ĵ�б��
	temp_k_more[0][0] = (left[RowMax - 30] - left[RowMax - 20]) / 10.0;//�ֱ�����С�ε�б��
	temp_k_more[0][1] = (left[RowMax - 20] - left[RowMax - 10]) / 10.0;
	temp_k_more[0][2] = (left[RowMax - 10] - left[RowMax]) / 10.0;

	temp_k[1] = (right[RowMax - (int)ZHI_NUM] - right[RowMax]) / ZHI_NUM;
	temp_k_more[1][0] = (right[RowMax - 30] - right[RowMax - 20]) / 10.0;
	temp_k_more[1][1] = (right[RowMax - 20] - right[RowMax - 10]) / 10.0;
	temp_k_more[1][2] = (right[RowMax - 10] - right[RowMax]) / 10.0;

	int i = 0;
	float YUZHI = 1;
	for (i = 1; i < (int)ZHI_NUM; i++)
	{
		float temp = 0;
		if ((left[RowMax] + int(temp_k[0] * i) - left[RowMax - i]) < YUZHI && (left[RowMax] + temp_k[0] * i - left[RowMax - i]) > -YUZHI)
			temp = 0;//ֱ�����Ե��i���괦���С����ֵ
		else
			temp = (left[RowMax] + temp_k[0] * i - left[RowMax - i]);//ֱ�����Ե��i���괦�Ĳ�
		temp_err[0] = temp_err[0] + temp;//�Բ������
	}
	for (i = 1; i < (int)ZHI_NUM; i++)
	{
		float temp = 0;
		if ((right[RowMax] + temp_k[1] * i - right[RowMax - i]) < YUZHI && (right[RowMax] + temp_k[1] * i - right[RowMax - i]) > -YUZHI)
			temp = 0;
		else
			temp = (right[RowMax] + temp_k[1] * i - right[RowMax - i]);
		temp_err[1] = temp_err[1] + temp;
	}
	//Բ���ĵ��Ĵ�ȷ��
	if (temp_err[0] > -10 && temp_err[0] < 10 && temp_err[1] > -10 && temp_err[1] < 10 && right[RowMax] != 79 && right[RowMax - 1] != 79 && right[RowMax - 2] != 79 && left[RowMax] > 2 && left[RowMax - 2] > 2 && left[RowMax - 3] > 2 && temp_k[0] > 0 && temp_k[1] < 0)
	{
		Ring_Confirm_forth = 1;
	}
	else ///��else�򻯼���
	{
		//("�޶�ֱ��");
		Ring_FIND = 0;
		return;
	}
	int *p_Ring_Confirm_third = &Ring_Confirm_third;
	//Բ������ȷ��
	uint8 res = Additional_Check_Ring(Ring_Curnor_Line[0], Ring_Curnor_Line[1], p_Ring_Confirm_third);

	if (Ring_Confirm_first == 1 && Ring_Confirm_second == 1 && Ring_Confirm_third == 1 && Ring_Confirm_forth == 1)
	{
		Ring_FIND = 1;
	}
	else if ((Ring_Confirm_third == 1 && Ring_Confirm_forth == 1))
	{
		if (res)
		{
			Ring_FIND = 1;
		}
	}
	else
	{
		Ring_FIND = 0;
	}
}
uint8 Is_line_all_white_or_black(uint8 line)
{
	uint8 this_lint_all_black = 1, this_line_all_white = 1;
	for (uint8 col = 0; col < 80; ++col)
	{
		if (img[line][col] == 0)
			this_line_all_white = 0;
		else
			this_lint_all_black = 0;
	}
	if (this_line_all_white)
		return 1;
	else if (this_lint_all_black)
		return 2;
	else
		return 0;
}
uint8 Additional_Check_Ring(int left_change_row, int right_change_row, int *p_Ring_Confirm_third)
{
	if (left_change_row > 38 || right_change_row > 38)
		return 0; //������̫��
	if (left_change_row < 15 || right_change_row < 15)
		return 0; //ɸѡ

	PosType leftP[60];
	PosType rightP[60];

	int cnt, line;

	for (line = RowMax, cnt = 0; line > left_change_row; line--)//��Ե��ѹ�ɽṹ����ʽ���ӵײ���Բ��
	{
		leftP[cnt].x = line;
		leftP[cnt].y = left[line];
		cnt++;
	}
	LeastSquareMethod(leftP, cnt);//��С���˷� ���k��d
	float k_local_left = k;
	float d_local_left = d;

	for (line = RowMax, cnt = 0; line > left_change_row; line--)//��Ե��ѹ�ɽṹ����ʽ���ӵײ���Բ��
	{
		rightP[cnt].x = line;
		rightP[cnt].y = right[line];
		cnt++;
	}
	LeastSquareMethod(rightP, cnt);//��С���˷� ���k��d
	float k_local_right = k;
	float d_local_right = d;

	/********************************�������������************************************************/
	int column;
	for (line = RowMax - 10; line >= RowMin + 1; line--)
	{
		int Ring_line_flag[2] = { 0 };
		for (column = left[line]; column <= right[line]; column++)//�������Ե���ұ�Ե
		{
			if (img[line][column] == THRESHOLD && img[line][column + 1] == 0)//��⵽�ұ�Ե���нǵ���߱�������
			{
				Ring_line_flag[0] = column;//���ұ�Ե�����긳ֵ��Ring_line_flag
				break;
			}
		}
		for (; column <= right[line]; column++)//�о��ж�������Ϊcolumn<80
		{
			if (img[line][column] == 0 && img[line][column + 1] == THRESHOLD)//���Բ�����н�
			{
				Ring_line_flag[1] = column;//�нǵ��ұ�������,һ�㲻��ִ�е����Ring_line_flag[1]һ��Ϊ0
				break;
			}
		}
		int temp = 0; //�������ƺ�Բ������λ��
		temp = (Ring_line_flag[0] + Ring_line_flag[1]) / 2;
		if ((Ring_line_flag[1] - Ring_line_flag[0]) > 20 && (Ring_line_flag[0] - left[line]) > 10 &&
			(right[line] - Ring_line_flag[1]) > 10)//(right[line] - Ring_line_flag[1])�нǵĿ���෴��
		{
			uint8 leftb = d_local_left + k_local_left * line; //��ϵı�Ե
			uint8 rightb = d_local_right + k_local_right * line;
			uint8 len = rightb - leftb;

			uint8 temp_max = rightb - len / 4;
			uint8 temp_min = leftb + len / 4;

			if (temp >= temp_min && temp <= temp_max)
			{
				*p_Ring_Confirm_third = 1;
				break; //Բ���Ĵ��Բ
			}
		}
	}

	if (*p_Ring_Confirm_third != 1 && line == RowMin) //���޻�
	{
		//("ʶ�𳬴�1");
		for (line = RowMin; line >= 0; --line)
		{
			int leftb = d_local_left + k_local_left * line;//��ϵı�Ե
			int rightb = d_local_right + k_local_right * line; //��ϵı�Ե
			cnt = rightb - leftb;
			if (cnt < 10)
				break;
			int cnt_confirm = 0;

			for (int col = leftb; col <= rightb; col++)//���Ե���ұ�Ե
			{
				if (img[line][col] == 0)
					cnt_confirm++;//ͼ��Ϊ��ʱ+1
			}

			if (abs(cnt_confirm - cnt) <= 2) //�м�ȫ�Ǻڣ�ͼ��Ϊ�ڵĵ�����ϵ�·��Ȳ�ࣩ
			{
				//("ʶ�𳬴�2");

				if (Super_Ring_Check(k_local_left, d_local_left, k_local_right, d_local_right))
				{
					*p_Ring_Confirm_third = 1;
					//("ʶ�𳬴�ok");
					return 1;
				}
				else
				{
					return 0;
				}
			}
		}
	}

	/********************************************************************************/
	int evidence = 0;
	for (line = RowMax; line > 5; line--)
	{
		int leftb = d_local_left + k_local_left * line;
		int rightb = d_local_right + k_local_right * line;
		cnt = rightb - leftb;
		if (cnt < 10)
			break;
		int cnt_confirm = 0;

		for (int col = leftb; col <= rightb; col++)
		{
			if (img[line][col] == 0)
				cnt_confirm++;
		}

		float temp = (float)cnt_confirm / (float)cnt;
		//("rate:%f\n", temp);
		if (temp > 0.9)
			evidence++;
	}
	//("evidence:%d", evidence);
	if (evidence >= 2)
		return 1;
	else
		return 0;
}