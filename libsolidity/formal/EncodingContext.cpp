/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <libsolidity/formal/EncodingContext.h>

#include <libsolidity/formal/SymbolicTypes.h>

using namespace std;
using namespace dev;
using namespace dev::solidity;
using namespace dev::solidity::smt;

EncodingContext::EncodingContext(SolverInterface& _solver):
	m_solver(_solver),
	m_thisAddress(make_unique<SymbolicAddressVariable>("this", m_solver))
{
	auto sort = make_shared<smt::ArraySort>(
		make_shared<smt::Sort>(smt::Kind::Int),
		make_shared<smt::Sort>(smt::Kind::Int)
	);
	m_balances = make_unique<SymbolicVariable>(sort, "balances", m_solver);
}

void EncodingContext::reset()
{
	m_thisAddress->increaseIndex();
	m_balances->increaseIndex();
	resetVariables([&](VariableDeclaration const&) { return true; });
}

bool EncodingContext::knownVariable(VariableDeclaration const& _varDecl)
{
	return m_variables.count(&_varDecl);
}

shared_ptr<SymbolicVariable> EncodingContext::variable(VariableDeclaration const& _varDecl)
{
	solAssert(knownVariable(_varDecl), "");
	return m_variables[&_varDecl];
}

bool EncodingContext::createVariable(VariableDeclaration const& _varDecl)
{
	solAssert(!knownVariable(_varDecl), "");
	auto const& type = _varDecl.type();
	auto result = newSymbolicVariable(*type, _varDecl.name() + "_" + to_string(_varDecl.id()), m_solver);
	m_variables.emplace(&_varDecl, result.second);
	return result.first;
}

void EncodingContext::resetVariable(VariableDeclaration const& _variable)
{
	newValue(_variable);
	setUnknownValue(_variable);
}

void EncodingContext::resetVariables(set<VariableDeclaration const*> const& _variables)
{
	for (auto const* decl: _variables)
		resetVariable(*decl);
}

void EncodingContext::resetVariables(function<bool(VariableDeclaration const&)> const& _filter)
{
	for_each(begin(m_variables), end(m_variables), [&](auto _variable)
	{
		if (_filter(*_variable.first))
			this->resetVariable(*_variable.first);
	});
}

smt::Expression EncodingContext::newValue(VariableDeclaration const& _decl)
{
	solAssert(knownVariable(_decl), "");
	return m_variables.at(&_decl)->increaseIndex();
}

void EncodingContext::setZeroValue(VariableDeclaration const& _decl)
{
	solAssert(knownVariable(_decl), "");
	setZeroValue(*m_variables.at(&_decl));
}

void EncodingContext::setZeroValue(SymbolicVariable& _variable)
{
	smt::setSymbolicZeroValue(_variable, m_solver);
}

void EncodingContext::setUnknownValue(VariableDeclaration const& _decl)
{
	solAssert(knownVariable(_decl), "");
	setUnknownValue(*m_variables.at(&_decl));
}

void EncodingContext::setUnknownValue(SymbolicVariable& _variable)
{
	smt::setSymbolicUnknownValue(_variable, m_solver);
}

smt::Expression EncodingContext::thisAddress()
{
	return m_thisAddress->currentValue();
}

smt::Expression EncodingContext::balance()
{
	return balance(m_thisAddress->currentValue());
}

smt::Expression EncodingContext::balance(smt::Expression _address)
{
	return smt::Expression::select(m_balances->currentValue(), move(_address));
}

void EncodingContext::transfer(smt::Expression _from, smt::Expression _to, smt::Expression _value)
{
	unsigned indexBefore = m_balances->index();
	addBalance(_from, 0 - _value);
	addBalance(_to, move(_value));
	unsigned indexAfter = m_balances->index();
	solAssert(indexAfter > indexBefore, "");
	m_balances->increaseIndex();
	/// Do not apply the transfer operation if _from == _to.
	auto newBalances = smt::Expression::ite(
		move(_from) == move(_to),
		m_balances->valueAtIndex(indexBefore),
		m_balances->valueAtIndex(indexAfter)
	);
	m_solver.addAssertion(m_balances->currentValue() == newBalances);
}

void EncodingContext::addBalance(smt::Expression _address, smt::Expression _value)
{
	auto newBalances = smt::Expression::store(
		m_balances->currentValue(),
		_address,
		balance(_address) + move(_value)
	);
	m_balances->increaseIndex();
	m_solver.addAssertion(newBalances == m_balances->currentValue());
}
