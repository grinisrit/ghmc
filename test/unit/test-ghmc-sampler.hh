#pragma once

#include "test-data.hh"

#include <noa/ghmc.hh>
#include <noa/utils/common.hh>

#include <gtest/gtest.h>

using namespace noa;
using namespace noa::utils;

inline const auto alog_funnel = [](const auto &ntheta) {
    auto dim = ntheta.numel() - 1;
    return -0.5 * ((torch::exp(ntheta[0]) * ntheta.slice(0, 1, dim + 1).pow(2).sum()) +
                   (ntheta[0].pow(2) / 9) - dim * ntheta[0]);
};

inline TensorOpt get_funnel_hessian(const torch::Tensor &theta, torch::DeviceType device)
{
    torch::manual_seed(utils::SEED);
    auto ntheta = theta.clone().to(device).requires_grad_();
    auto log_prob = alog_funnel(ntheta);
    return numerics::hessian(log_prob, ntheta);
}

inline void test_funnel_hessian(torch::DeviceType device = torch::kCPU)
{
    auto hess = get_funnel_hessian(GHMCData::get_theta(), device);

    ASSERT_TRUE(hess.has_value());
    ASSERT_TRUE(hess.value().device().type() == device);
    auto res = hess.value().detach().to(torch::kCPU);
    auto err = (res + GHMCData::get_neg_hessian_funnel()).abs().sum().item<float>();
    ASSERT_NEAR(err, 0., 1e-3);
}

inline ghmc::FisherInfo get_fisher_info(const torch::Tensor &theta, torch::DeviceType device)
{
    torch::manual_seed(utils::SEED);
    auto ntheta = theta.clone().to(device).requires_grad_();
    auto log_prob = alog_funnel(ntheta);
    return ghmc::fisher_info(log_prob, ntheta);
}

inline ghmc::SymplecticFlow get_symplectic_flow(
    const torch::Tensor &theta, 
    const torch::Tensor &momentum,
    torch::DeviceType device)
{
    torch::manual_seed(utils::SEED);
    auto ntheta = theta.clone().to(device);
    auto nmomentum = momentum.clone().to(device);

    return ghmc::symplectic_flow(/*log_probability_density=*/alog_funnel,
                                 /*params=*/ntheta, /*nmomentum=*/nmomentum,
                                 /*leap_steps=*/1, /*epsilon=*/0.14, /*binding_const=*/10.,
                                 /*jitter=*/0.00001, /*jitter_max=*/0);
}

inline ghmc::SoftAbsMap get_metric(const torch::Tensor &theta)
{
    torch::manual_seed(utils::SEED);
    auto ntheta = theta.clone().requires_grad_();
    auto log_prob = alog_funnel(ntheta);
    return ghmc::softabs_map(log_prob, ntheta, 0.);
}

inline ghmc::Hamiltonian get_hamiltonian(
    const torch::Tensor &theta,
    const torch::Tensor &momentum)
{
    torch::manual_seed(utils::SEED);
    auto ntheta = theta.clone().requires_grad_();
    auto nmomentum = momentum.clone().requires_grad_();
    return ghmc::hamiltonian(alog_funnel, ntheta, nmomentum, 0.);
}

inline void test_fisher_info(torch::DeviceType device = torch::kCPU)
{
    auto fisher = get_fisher_info(GHMCData::get_theta(), device);

    ASSERT_TRUE(fisher.has_value());
    ASSERT_TRUE(fisher.value().device().type() == device);
    auto res = fisher.value().to(torch::kCPU);
    auto err = (res - GHMCData::get_neg_hessian_funnel()).abs().sum().item<float>();
    ASSERT_NEAR(err, 0., 1e-3);
}

inline void test_symplectic_flow(torch::DeviceType device = torch::kCPU)
{
    auto flow = get_symplectic_flow(GHMCData::get_theta(), GHMCData::get_momentum(), device);

    ASSERT_TRUE(flow.has_value());
    auto [p_flow_, m_flow_] = flow.value();
    ASSERT_TRUE(p_flow_.device().type() == device);
    auto p_flow = p_flow_.to(torch::kCPU);
    ASSERT_TRUE(m_flow_.device().type() == device);
    auto m_flow = m_flow_.to(torch::kCPU);
    auto err = (p_flow[-1] - GHMCData::get_expected_flow_theta()).abs().sum().item<float>();
    ASSERT_NEAR(err, 0., 1e-2);
    err = (m_flow[-1] - GHMCData::get_expected_flow_moment()).abs().sum().item<float>();
    ASSERT_NEAR(err, 0., 1e-2);
}