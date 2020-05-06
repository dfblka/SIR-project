#include <algorithm>
#include <cassert>
#include <chrono>
#include <iostream>
#include <iterator>
#include <random>
#include <stdexcept>
#include <thread>
#include <vector>

struct Position {
  int i;
  int j;
};
struct Sir {
  double s;
  double i;
  double r;
};

enum class State : char { S, I, R };

class Board {
  int length_;
  int total_;
  std::vector<State> b_{};
  std::vector<Sir> Sir_expected_;
  double beta_ = 0;
  double gamma_ = 0;

public:
  Board(int length, double infected, double Ro, double gamma)
      : length_{length}, total_{length_ * length_}, b_(total_, State::S),
        Sir_expected_{{total_ - infected, infected, 0.}}, gamma_{gamma} {
    int size = Sir_expected_.size();
    assert(size == 1);
    beta_ = gamma_ * Ro / (total_ - infected);
    assert(gamma_ > 0 && gamma_ < 1);
    assert(size == 1);
    //we choose by random lib "infected" position to be infected
    std::mt19937 gen_i ;
    std::mt19937 gen_j ;
    std::uniform_int_distribution<> roll_dice (0 , length_ - 1) ;
    for ( int n = 0 ; n != infected ; ++n) {
      int i = roll_dice(gen_i) ;
      int j = roll_dice(gen_j) ;
      assert(i >= 0 && i < length_ && j >= 0 && j < length_) ;
      b_[length_*i + j] = State::I ;
    }
  }

  int get_length() const & { return length_; }
  double get_beta() const & { return beta_; }
  double get_gamma() const & { return gamma_; }

  State operator()(int i, int j) const { // start from 00
    return (i >= 0 && i < length_ && j >= 0 && j < length_)
               ? b_[i * length_ + j]
               : State::S ;
  }

  State &operator()(int i, int j) { // start from 00
    assert(i >= 0 && i <= length_ && j >= 0 && j <= length_);
    int n = b_.size();
    return b_[i * n + j];
  }

  Sir const get_condition(int i, int j) const & {
    if (i < 0 || i > length_ - 1 || j < 0 || j > length_ - 1) {
      throw std::runtime_error{"Out of Board"};
    } else {
      Sir n{};
      int i_near = i - 1;
      int j_near = j - 1;
      if (i == 0 || j == 0 || i == length_ - 1 || j == length_ - 1) {
        for (; i_near != i + 2; ++i_near) {
          for (; i_near != i + 2; ++i_near) {
            if (i_near >= 0 || j_near >= 0 || i_near < length_ ||
                j_near < length_) {
              if (b_[i * length_ * length_ + j] == State::S) {
                ++(n.s);
              }
              if (b_[i * length_ * length_ + j] == State::I) {
                ++(n.i);
              }
              if (b_[i * length_ * length_ + j] == State::R) {
                ++(n.r);
              }
            }
          }
        }
      } else {
        for (; i_near != i + 2; ++i_near) {
          for (; j_near != j + 2; ++j_near) {
            if (b_[i * length_ * length_ + j] == State::S) {
              ++(n.s);
            }
            if (b_[i * length_ * length_ + j] == State::I) {
              ++(n.i);
            }
            if (b_[i * length_ * length_ + j] == State::R) {
              ++(n.r);
            }
          }
        }
      }
      return n;
    }
  }

  Sir const sir_evolution() & {

    Sir last = *(Sir_expected_.end() - 1);

    double s_state = last.s - (beta_ * last.i * last.s);
    double i_state = last.i + (beta_ * last.i * last.s) - gamma_ * last.i;
    double r_state = last.r + gamma_ * last.i;

    /* if (s_state <= 0) {
      s_state = 0;
    }
    if (r_state >= total_ ) {
        r_state = total_;
    }
    if (s_state == 0 && r_state == total_) {
        i_state = 0;
    }
    */
    double sum = s_state + i_state + r_state;
    assert(sum > (total_ - 0.01 * total_) && sum < (total_ + 0.01 * total_));

    Sir expected{s_state, i_state, r_state};
    Sir_expected_.push_back(expected);
    return expected;
  }
};

Board const evolve(Board const &b) {
  Board br = b;
  std::vector<std::vector<Position>> Infectable(8);
  for (int i = 0; i != br.get_length(); ++i) {
    for (int j = 0; j != br.get_length(); ++j) {
      Sir near = b.get_condition(i, j);
      for (int n = 0; n != 8; ++n) {
        if (b(i, j) == State::S && near.i == n + 1) {
          (Infectable[n]).push_back({i, j});
        }
      }
    }
  }
  // i nuovi infetti che vado a disporre devono essere in accordo con expected.i
  // i nuovi infetti vanno assegnati in base al numero di infetti che stanno
  // attorno al suscettibile
  Sir expected = br.sir_evolution();
  std::random_device rd;
  std::mt19937 g(rd());
  for (int n = 0; n != 8; ++n) {
    std::shuffle((Infectable[n]).begin(), Infectable[n].end(), g);
  }
  int infected = expected.i; // in order to work with integer values

  for (int n = 7; n >= 0; --n) {
    if (infected == 0) {
      break;
    }
    for (Position const &p : Infectable[n]) {
      if (infected == 0 || Infectable[n].size() == 0) {
        break;
      }
      br(p.i, p.j) = State::I;
      --infected;
    }
  }
  assert(Infectable.size() == 8);
  return br;
}

void print(Board const &b) {
  std::cout << ' ' + std::string(b.get_length() + 2, '#') << '\n';
  for (int i = 1; i != b.get_length() + 1; ++i) {
    std::string line = " #";
    for (int j = 1; j != b.get_length() + 1; ++j) {
      if (b(i, j) == State::I) {
        line += 'X';
      } else if (b(i, j) == State::S) {
        line += ' ';
      } else {
        line += 'O';
      }
    }
    line += '#';

    std::cout << line << '\n';
  }
  std::cout << ' ' + std::string(b.get_length() + 2, '#') << '\n';
};

void animate(Board &b, int n_evol) {
  using namespace std::chrono_literals;
  print(b);
  std::this_thread::sleep_for(500ms);
  std::cout << "\033c";
  for (int n = 1; n != n_evol; ++n) {
    b = evolve(b);
    print(b);
    std::this_thread::sleep_for(500ms);
    std::cout << "\033c";
  }
  std::cout << "\033c";
  print(evolve(b));
};

int main() {
  Board b{25, 1., 2., 0.5};
  animate(b, 10);
}