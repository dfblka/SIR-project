
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cmath>

struct State {
  int time;
  double s;
  double i;
  double r;
};

enum class Epidemics { e1, e2, e3 };

class sir {
private:
  std::vector<State> lines_;

  double beta_ = 0;
  double gamma_ = 0;
  double s_zero_ = 0; 
  double i_zero_; 
  double r_zero_ = 0; 
  double total_;  // total population

public:
//First constructor
  sir(double total, double infected, double R0, double gamma)
      : gamma_{gamma}, i_zero_{infected}, total_{total} {
    assert(gamma > 0 && gamma <= 1); 
    s_zero_ = total_ - i_zero_; 
    beta_= gamma_ * R0 / s_zero_;
    assert(gamma_> 0 && gamma_ < 1);
  }
//Second constructor
  sir(Epidemics e, double total, double infected) : i_zero_{infected}, total_{total} {
    switch (e) {
    case Epidemics::e1: {
      beta_ = 0.002;  
      gamma_ = 0.5;  //with a fixed R0 = 2
      s_zero_ = total_ -i_zero_ ; 
    }; break;
    default:
      throw std::runtime_error{"Invalid set"};
    }
  }
  //
  void graph(int const &time) { // time express how many time unities to graph
    assert(s_zero_ != 0 && beta_ != 0 &&
           gamma_ != 0); // to ensure te costructor has been called
    State zero{0, s_zero_, i_zero_, r_zero_}; // 0, 1000-1 , 1
    lines_.push_back(zero);

    for (int last = 0; last != time; ++last) {

      double s_state =
          (lines_[last]).s - (beta_ * (lines_[last]).i * (lines_[last]).s);
      double i_state = (lines_[last]).i +
                       (beta_ * (lines_[last]).i * (lines_[last]).s) -
                       gamma_ * (lines_[last]).i;
      double r_state = (lines_[last]).r + gamma_ * (lines_[last]).i;

      if (s_state <= 0) {
        s_state = 0;      
      }
      if (r_state >= total_ ) {
          r_state = total_;
      }
      if (s_state == 0 && r_state == total_) {
          i_state = 0;
      }

      double sum = s_state + i_state + r_state;
      assert(sum > (total_ - 0.01 * total_) &&
               sum < (total_ + 0.01 * total_));


      lines_.push_back({last + 1, s_state, i_state, r_state});
      
      //assert
      /*double right_compare = ( 1 - gamma_ /( s_state * beta_) ) ;
      assert( ( ( i_state + r_state ) / total_ ) <  ( to_compare + to_compare*0.01 ) 
      &&
      ( ( i_state + r_state ) / total_ ) > ( to_compare - to_compare*0.01 ) );*/
    }
  }
  void print() const & {

    std::cout << "Time   "
              << "S   "
              << "I   "
              << "R   "
              << "\n";
    for (int i = 0; i != static_cast<int>(lines_.size()); ++i) {
      std::cout << (lines_[i]).time << "  " << (lines_[i]).s << "  "
                << (lines_[i]).i << "  " << (lines_[i]).r << "\n";
    }
  }
};

int main() {
  sir S{625 , 1, 2, .05};
  //sir S{Epidemics::e1};
  S.graph(100);
  S.print();
  std::cout << "\n" << "\n" ;
  /*sir S2{Epidemics::e1, 500 ,1} ;
  S2.graph(100);
  S2.print(); */ 
}
