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
/**
 * Optimiser component that combines syntactically equivalent functions.
 */

#include <libyul/optimiser/BlockClassFinder.h>
#include <libyul/optimiser/SyntacticalEquality.h>
#include <libdevcore/CommonData.h>

using namespace std;
using namespace dev;
using namespace yul;

namespace {
static constexpr uint64_t literalHash(char const *_literal, size_t _N) {
	return (_N == 0) ? 14695981039346656037u : literalHash(_literal + 1, _N - 1) ^ 1099511628211u;
}

template<size_t N>
static constexpr uint64_t literalHash(char const (&_literal)[N]) {
	return literalHash(_literal, N);
}
}

bool BlockClassFinder::isExternal(YulString _identifier) const
{
	auto it = m_identifierMapping.find(_identifier);
	yulAssert(it != m_identifierMapping.end(), "");
	return (it->second & 1) == 0;
}

std::vector<BlockClass> BlockClassFinder::run(Block const& _block)
{
	GlobalState result;
	BlockClassFinder blockClassFinder(result);
	blockClassFinder(_block);
	return std::move(result.blockClasses);
}

void BlockClassFinder::operator()(Literal const& _literal)
{
	hash(literalHash("Literal"));
	hash(_literal.value.hash());
	hash(_literal.type.hash());
	hash(static_cast<std::underlying_type_t<LiteralKind>>(_literal.kind));
}

void BlockClassFinder::operator()(Identifier const& _identifier)
{
	hash(literalHash("Identifier"));
	size_t id = 0;
	auto it = m_identifierMapping.find(_identifier.name);
	if (it == m_identifierMapping.end())
	{
		id = 2 * (m_externalIdentifierCount++);
		m_identifierMapping[_identifier.name] = id;
		m_externalIdentifiers.emplace_back(_identifier.name);
	}
	else
		id = it->second;
	if ((id & 1) == 0)
	{
		if (m_isAssignmentLHS)
			m_externalAssignments.insert(_identifier.name);
		else
			m_externalReads.insert(_identifier.name);
	}
	hash(id);
}

void BlockClassFinder::operator()(FunctionalInstruction const& _instr)
{
	hash(literalHash("FunctionalInstruction"));
	hash(static_cast<std::underlying_type_t<eth::Instruction>>(_instr.instruction));
	ASTWalker::operator()(_instr);
}

void BlockClassFinder::operator()(FunctionCall const& _funCall)
{
	hash(literalHash("FunctionCall"));
	hash(_funCall.arguments.size());
	hash(_funCall.functionName.name.hash());
	ASTWalker::operator()(_funCall);
}

void BlockClassFinder::operator()(ExpressionStatement const& _statement)
{
	hash(literalHash("ExpressionStatement"));
	ASTWalker::operator()(_statement);
}

void BlockClassFinder::operator()(Assignment const& _assignment)
{
	hash(literalHash("Assignment"));
	hash(_assignment.variableNames.size());
	m_isAssignmentLHS = true;
	for (auto const& name: _assignment.variableNames)
		(*this)(name);
	m_isAssignmentLHS = false;
	visit(*_assignment.value);
}

void BlockClassFinder::operator()(VariableDeclaration const& _varDecl)
{
	hash(literalHash("VariableDeclaration"));
	hash(_varDecl.variables.size());
	for (auto const& var: _varDecl.variables)
	{
		yulAssert(!m_identifierMapping.count(var.name), "");
		m_identifierMapping[var.name] = 2 * m_internalIdentifierCount + 1;
	}
	ASTWalker::operator()(_varDecl);
}

void BlockClassFinder::operator()(If const& _if)
{
	hash(literalHash("If"));
	ASTWalker::operator()(_if);
}

void BlockClassFinder::operator()(Switch const& _switch)
{
	hash(literalHash("Switch"));
	hash(_switch.cases.size());
	ASTWalker::operator()(_switch);
}

void BlockClassFinder::operator()(FunctionDefinition const& _funDef)
{
	// TODO: It might be enough to visit the body only in a sub-BlockClassFinder.
	hash(literalHash("FunctionDefinition"));
	ASTWalker::operator()(_funDef);
}

void BlockClassFinder::operator()(ForLoop const& _loop)
{
	hash(literalHash("ForLoop"));
	ASTWalker::operator()(_loop);
}

void BlockClassFinder::operator()(Break const& _break)
{
	hash(literalHash("Break"));
	ASTWalker::operator()(_break);
}

void BlockClassFinder::operator()(Continue const& _continue)
{
	hash(literalHash("Break"));
	ASTWalker::operator()(_continue);
}


void BlockClassFinder::operator()(Block const& _block)
{
	hash(literalHash("Block"));
	BlockClassFinder subBlockClassFinder(m_globalState);
	for (auto const& statement: _block.statements)
		subBlockClassFinder.visit(statement);

	hash(subBlockClassFinder.m_hash);
	for (auto const& externalIdentifier: subBlockClassFinder.m_externalIdentifiers)
		(*this)(Identifier{{}, externalIdentifier});
	for (auto const& externalAssignment: subBlockClassFinder.m_externalAssignments)
		if (isExternal(externalAssignment))
			m_externalAssignments.insert(externalAssignment);
	for (auto const& externalAssignment: subBlockClassFinder.m_externalReads)
		if (isExternal(externalAssignment))
			m_externalReads.insert(externalAssignment);

	auto& candidateIDs = m_globalState.hashToBlockIDs[subBlockClassFinder.m_hash];

	// look for existing block class
	for (auto const& candidateID: candidateIDs)
	{
		auto const& candidate = m_globalState.block(candidateID);
		if (subBlockClassFinder.m_externalIdentifiers.size() == candidate.externalReferences.size())
		{
			if (
				SyntacticallyEqual{
					subBlockClassFinder.m_externalIdentifiers,
					candidate.externalReferences
				}.statementEqual(_block, *candidate.block)
			)
			{
				m_globalState.blockToClassID[&_block] = candidateID.blockClass;
				m_globalState.blockClasses[candidateID.blockClass].emplace_back(BlockClassMember{
					&_block,
					std::move(subBlockClassFinder.m_externalIdentifiers),
					std::move(subBlockClassFinder.m_externalAssignments),
					std::move(subBlockClassFinder.m_externalReads)
				});
				return;
			}
		}
	}

	// create new block class
	candidateIDs.emplace_back(GlobalState::BlockID{m_globalState.blockClasses.size(), 0});
	m_globalState.blockToClassID[&_block] = m_globalState.blockClasses.size();
	m_globalState.blockClasses.emplace_back(make_vector<BlockClassMember>(
		std::forward_as_tuple(
			&_block,
			std::move(subBlockClassFinder.m_externalIdentifiers),
			std::move(subBlockClassFinder.m_externalAssignments),
			std::move(subBlockClassFinder.m_externalReads)
		)
	));
}
