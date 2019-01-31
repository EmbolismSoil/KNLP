#ifndef KNLP_LBFGS_H
#define KNLP_LBFGS_H

#include <funtional>
#include <Eigen/Dense>

template<class ...Args>
class LBFGS{
public:
    using grad_callable_type = std::function<Eigen::MatrixXd(Eigen::MatrixXd, Args &&...args)>;
    using forward_callable_type = std::function<std::double_t(Eigen::MatrixXd, Args &&...args)>;

    LBFGS(grad_callable_type grad, forward_callable_type forward, Eigen::MatrixXd init_paramters):
        _grad(grad),
        _forward(forward),
        _paramters(init_paramters)
    {

    }

    void update(Args &&...args)
    {
        auto gt = _grad(_paramters, args...);
        auto g_delta = gt - _last_grad;
        //注意行列优化
        auto paramters_delta = -_h * g_delta;
        //line search r
        auto lambda = _line_search(paramters_delta, args...);
        paramters_delta *= lambda;
        //update
        
        auto I = Eigen::Identify();
        _h = (I - (paramters_delta*g_delta.transpose())/(g_delta.transpose()*paramters_delta))*_h
             *(I - (g_delta*paramters_delta.transpose())/(y.transpose()*paramters_delta)) 
             + (g_delta*g_delta.transpose())/(paramters_delta.transpose()*g_delta);
    }

    Eigen::MatrixXd error()
    {

    }

private:
    grad_callable_type _grad;
    forward_callable_type _forward;
    Eigen::MatrixXd _paramters;
    Eigen::MatrixXd _last_grad;
    Eigen::MatrixXd _h;

    std::double_t _line_search(Eigen::MatrixXd paramters_delta, Args... &&args)
    {

    }
};
#endif
