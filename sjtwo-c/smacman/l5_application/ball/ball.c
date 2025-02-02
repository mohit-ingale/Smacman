
#include "ball_graphic.h"

// bool blue_pacman_start = 1, green_pacman_start = 1;
gpio_s switch0, switch1, switch2, switch3;

// Columns - along X-axis, Rows - along Y-axis
void ball_task(void *params) {

  game_logic_game_state_s change_game_state_to;
  ball_s ball;
  ball_param ball_parameters = *(ball_param *)params;
  static uint8_t count = 1;
  ball_setup(&ball);
  uint8_t score_blue = 0, score_green = 0;

  uint8_t cummulative_score = 1;
  bool blue_collided, green_collided;
  while (1) {
    // led_matrix_clear_frame_buffer_inside_grid(ALL_INSIDE_GRID);
    led_matrix__drawBall(ball.row, ball.col, OFF);
    count = (count < 4) ? count + 1 : 1;

    // Update x movement // Wall collision
    if (count % (5 - ball.vx) == 0) {
      ball.row = (ball.xDir > 0) ? ball.row + 1 : ball.row - 1; // left: right
      // If wall collision then reverse the x direction
      if (ball.row <= 4 || ball.row >= (matrix_width - 5)) {
        ball.xDir = -ball.xDir;
      }
    }

    if (count % (5 - ball.vy) == 0) {
      ball.col = (ball.yDir > 0) ? ball.col + 1 : ball.col - 1; // up:down

      // Paddle Collision

      // bool left_blue = gpio__get(switch0);
      // bool right_blue = gpio__get(switch1);
      // bool left_green = gpio__get(switch2);
      // bool right_green = gpio__get(switch3);

      bool left_blue = false;
      bool right_blue = false;
      bool left_green = false;
      bool right_green = false;
      paddle_direction_e green_pacman_direction;
      paddle_direction_e blue_pacman_direction;
      xQueueReceive(blue_paddle_direction_queue, &blue_pacman_direction, 0);
      xQueueReceive(green_paddle_direction_queue, &green_pacman_direction, 0);
      // SMACMAN__DEBUG_PRINTF("Direction for Blue Paddle is %i\n", blue_pacman_direction);
      if (blue_pacman_direction == PADDLE_DIRECTION_LEFT) {
        left_blue = true;
      } else if (blue_pacman_direction == PADDLE_DIRECTION_RIGHT) {
        right_blue = true;
      }
      if (green_pacman_direction == PADDLE_DIRECTION_LEFT) {
        left_green = true;
      } else if (green_pacman_direction == PADDLE_DIRECTION_RIGHT) {
        right_green = true;
      }

      if (ball.col == 6) {
        if ((led_matrix__get_pixel(ball.row, 3) != 0) || (led_matrix__get_pixel(ball.row - 1, 3) != 0) ||
            (led_matrix__get_pixel(ball.row + 1, 3) != 0)) { // covers 3 rows
          // SMACMAN__DEBUG_PRINTF("down : %d, row: %d \n", led_matrix__get_pixel(ball.row, 3), ball.row);
          if ((right_blue && ball.xDir < 0) || (left_blue && ball.xDir > 0)) {
            increase_ball_x(&ball);
          }
          if ((right_blue && ball.xDir > 0) || (left_blue && ball.xDir < 0)) {
            decrease_ball_x(&ball);
          }
          ball.yDir = -ball.yDir;
          cummulative_score++;
          SMACMAN__DEBUG_PRINTF("Score to grab: %d \n", cummulative_score);
        }
      }
      // oppponent scores
      if (ball.col == 3) {
        score_green += cummulative_score;
        set_players_score(score_green, score_blue);
        // set_game_state(IN_SCORE_STATE);
        change_game_state_to = IN_SCORE_STATE;
        xQueueSend(*(ball_parameters.state_queue), &change_game_state_to, 0);
        // display_score(score_green, score_blue);
        cummulative_score = 1;
        // led_matrix__cleanBall(ball.row - 1, ball.col - 1);
        SMACMAN__DEBUG_PRINTF("GREEN player scored\n");
        SMACMAN__DEBUG_PRINTF("BLUE player score: %d   GREEN player score: %d\n", score_blue, score_green);
        set_blue_pacman_start();
        set_green_pacman_start();
        ball_setup(&ball);
      }
      if (ball.col == matrix_width - 7) {
        if ((led_matrix__get_pixel(ball.row, matrix_width - 4) != 0) ||
            (led_matrix__get_pixel(ball.row - 1, matrix_width - 4) != 0) ||
            (led_matrix__get_pixel(ball.row + 1, matrix_width - 4) != 0)) {
          // SMACMAN__DEBUG_PRINTF("up : %d,  row: %d \n", led_matrix__get_pixel(ball.row, matrix_width - 4), ball.row);
          if ((right_green && ball.xDir < 0) || (left_green && ball.xDir > 0)) {
            increase_ball_x(&ball);
          }
          if ((right_green && ball.xDir > 0) || (left_green && ball.xDir < 0)) {
            decrease_ball_x(&ball);
          }
          ball.yDir = -ball.yDir;
          cummulative_score++;
          SMACMAN__DEBUG_PRINTF("Score to grab: %d \n", cummulative_score);
        }
      }

      if (ball.col == matrix_width - 4) {
        // led_matrix__cleanBall(ball.row - 1, ball.col - 1);
        score_blue += cummulative_score;
        set_players_score(score_green, score_blue);
        change_game_state_to = IN_SCORE_STATE;
        xQueueSend(*(ball_parameters.state_queue), &change_game_state_to, 0);
        // set_game_state(IN_SCORE_STATE);
        // display_score(score_green, score_blue);
        cummulative_score = 1;
        SMACMAN__DEBUG_PRINTF("BLUE player scored\n");
        SMACMAN__DEBUG_PRINTF("BLUE player score: %d   GREEN player score: %d\n", score_blue, score_green);
        set_blue_pacman_start();
        set_green_pacman_start();
        ball_setup(&ball);
      }
    }

    // if ((ball.col == matrix_width / 2 - 2) || (ball.col == matrix_width / 2 + 2) && (ball_parameters.level != 3)) {
    //   // printf("Setting ball position\n");
    //   set_ball_position_direction(&ball);
    // }
    if (ball.col > 0) {
      set_ball_position_direction(&ball);
    }

    detect_collision(ball);
    blue_collided = get_blue_collided();
    green_collided = get_green_collided();
    if (blue_collided == 1) {
      blue_collided = clear_blue_collided();
      score_blue += cummulative_score;
      set_players_score(score_green, score_blue);
      change_game_state_to = IN_SCORE_STATE;
      xQueueSend(*(ball_parameters.state_queue), &change_game_state_to, 0);
      // set_game_state(IN_SCORE_STATE);
      // display_score(score_green, score_blue);
      cummulative_score = 1;
      SMACMAN__DEBUG_PRINTF("BLUE player score: %d   GREEN player score: %d\n", score_blue, score_green);
      set_blue_pacman_start();
      set_green_pacman_start();
      ball_setup(&ball);
    }
    if (green_collided == 1) {
      green_collided = clear_green_collided();
      score_green += cummulative_score;
      set_players_score(score_green, score_blue);
      change_game_state_to = IN_SCORE_STATE;
      xQueueSend(*(ball_parameters.state_queue), &change_game_state_to, 0);

      // set_game_state(IN_SCORE_STATE);

      // display_score(score_green, score_blue);
      // led_matrix__fill_frame_buffer_inside_grid();
      cummulative_score = 1;
      SMACMAN__DEBUG_PRINTF("BLUE player score: %d   GREEN player score: %d\n", score_blue, score_green);
      set_blue_pacman_start();
      set_green_pacman_start();
      ball_setup(&ball);
    }
    if (cummulative_score == 2) {
      set_start_detection();
    }
    if (cummulative_score == 1) {
      clear_start_detection();
    }
    vTaskDelay(25);
  }
}
